#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif

#include "audio.h"

#include "third_party/miniaudio/miniaudio_config.h"
#include "third_party/miniaudio/miniaudio.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Bộ trộn tự viết trên ma_device: mỗi hiệu ứng là một mảng mẫu mono (float, 44.1 kHz) tạo sẵn lúc
// khởi động; callback âm thanh cộng các "voice" đang phát + nhạc nền lặp vô hạn.

namespace {

const int SR = 44100;
typedef std::vector<float> Buf;
const double TWO_PI = 6.283185307179586;

const char *SFX_NAMES[SFX_COUNT] = {
    "roll", "stand", "switch", "heavy", "bridge_on", "bridge_off", "fall",
    "win", "split", "merge", "crack", "click", "start", "collapse"
};

struct Voice { const Buf *buf; double pos; float gain, rate; };

ma_device device;
bool device_ok = false;
std::mutex mtx;
Buf sfx[SFX_COUNT];
Buf music;
size_t music_pos = 0;
float music_fade = 0.0f;          // tăng dần từ 0 -> 1 lúc mở game
float vol_music = 0.35f, vol_sfx = 0.8f;
std::vector<Voice> voices;
std::thread music_thread;
std::string status_text = "off";

// ------------------------------------------------------------------ tổng hợp
struct Rng
{
    uint32_t s;
    explicit Rng(uint32_t seed) : s(seed) {}
    float next() { s = s * 1664525u + 1013904223u; return float((s >> 8) & 0xFFFF) / 32768.0f - 1.0f; }
};

enum Wave { SINE, TRI, SQUARE, SAW };

float osc(double phase, int wave)
{
    double p = phase - std::floor(phase);
    switch(wave)
    {
        case TRI:    return float(4.0 * std::fabs(p - 0.5) - 1.0);
        case SQUARE: return p < 0.5 ? 0.6f : -0.6f;
        case SAW:    return float(2.0 * p - 1.0) * 0.7f;
        default:     return float(std::sin(TWO_PI * p));
    }
}

Buf make(double seconds) { return Buf(size_t(seconds * SR) + 1, 0.0f); }

// Nốt có trượt cao độ f0 -> f1 (theo hàm mũ), bao âm attack + tắt dần exp(-decay t).
// wrap = true: ghi vòng (cho nhạc nền lặp liền mạch).
void tone(Buf &b, double t0, double dur, double f0, double f1, float amp, double decay,
          int wave = SINE, double attack = 0.004, bool wrap = false)
{
    size_t start = size_t(t0 * SR), n = size_t(dur * SR);
    double phase = 0.0;
    bool glide = std::fabs(f1 / f0 - 1.0) > 1e-9;
    for(size_t k = 0; k < n; k++)
    {
        size_t i = start + k;
        if(i >= b.size()) { if(!wrap) break; i %= b.size(); }
        double t = double(k) / SR;
        double f = glide ? f0 * std::pow(f1 / f0, t / dur) : f0;
        phase += f / SR;
        double env = (t < attack ? t / attack : 1.0) * std::exp(-t * decay);
        double tail = dur - t;
        if(tail < 0.01) env *= tail / 0.01;
        b[i] += float(amp * osc(phase, wave) * env);
    }
}

// Tiếng ồn lọc thông thấp một cực; lp0 -> lp1 là hệ số lọc đầu/cuối (nhỏ = trầm).
void noise(Buf &b, double t0, double dur, float amp, double decay, double lp0, double lp1,
           uint32_t seed, bool highpass = false, bool wrap = false)
{
    Rng r(seed);
    size_t start = size_t(t0 * SR), n = size_t(dur * SR);
    double y = 0.0;
    for(size_t k = 0; k < n; k++)
    {
        size_t i = start + k;
        if(i >= b.size()) { if(!wrap) break; i %= b.size(); }
        double t = double(k) / SR;
        double lp = lp0 + (lp1 - lp0) * (t / dur);
        double x = r.next();
        y += lp * (x - y);
        double v = highpass ? x - y : y;
        double env = std::exp(-t * decay) * std::fmin(1.0, t / 0.002);
        b[i] += float(amp * v * env);
    }
}

void normalize(Buf &b, float peak)
{
    float m = 1e-6f;
    for(size_t i = 0; i < b.size(); i++) m = std::fmax(m, std::fabs(b[i]));
    for(size_t i = 0; i < b.size(); i++) b[i] *= peak / m;
}

double midi(int note) { return 440.0 * std::pow(2.0, (note - 69) / 12.0); }

void build_sfx()
{
    // Lăn / nằm xuống: tiếng "cộp" gỗ trầm.
    sfx[SFX_ROLL] = make(0.22);
    tone(sfx[SFX_ROLL], 0, 0.20, 120, 62, 0.9f, 22);
    noise(sfx[SFX_ROLL], 0, 0.06, 0.5f, 60, 0.25, 0.05, 11);
    tone(sfx[SFX_ROLL], 0, 0.03, 900, 500, 0.15f, 120, TRI);
    // Dựng đứng: "cốc" cao hơn, gọn hơn.
    sfx[SFX_STAND] = make(0.22);
    tone(sfx[SFX_STAND], 0, 0.18, 180, 95, 0.9f, 26);
    noise(sfx[SFX_STAND], 0, 0.05, 0.45f, 70, 0.35, 0.08, 12);
    tone(sfx[SFX_STAND], 0, 0.035, 1400, 800, 0.2f, 110, TRI);
    // Công tắc thường: "tách - cách".
    sfx[SFX_SWITCH] = make(0.18);
    tone(sfx[SFX_SWITCH], 0.00, 0.03, 2200, 1800, 0.5f, 90, SQUARE);
    tone(sfx[SFX_SWITCH], 0.05, 0.05, 1100, 900, 0.45f, 60, SQUARE);
    // Công tắc nặng: tiếng kim loại nặng.
    sfx[SFX_HEAVY] = make(0.7);
    tone(sfx[SFX_HEAVY], 0, 0.25, 90, 60, 0.9f, 14);
    const double partial[4] = {523, 871, 1330, 1990};
    for(int k = 0; k < 4; k++) tone(sfx[SFX_HEAVY], 0, 0.65, partial[k], partial[k] * 0.99, 0.18f / (k + 1), 6 + 2 * k);
    noise(sfx[SFX_HEAVY], 0, 0.05, 0.4f, 70, 0.5, 0.2, 13);
    // Cầu hiện ra / biến mất: quét lên / xuống.
    sfx[SFX_BRIDGE_ON] = make(0.45);
    tone(sfx[SFX_BRIDGE_ON], 0, 0.40, 260, 880, 0.5f, 4, TRI, 0.02);
    tone(sfx[SFX_BRIDGE_ON], 0.08, 0.32, 520, 1760, 0.18f, 6, SINE, 0.02);
    sfx[SFX_BRIDGE_OFF] = make(0.4);
    tone(sfx[SFX_BRIDGE_OFF], 0, 0.36, 700, 180, 0.5f, 5, TRI, 0.01);
    noise(sfx[SFX_BRIDGE_OFF], 0, 0.3, 0.15f, 8, 0.2, 0.02, 14);
    // Rơi: tiếng "vút" xuống thấp dần.
    sfx[SFX_FALL] = make(1.1);
    tone(sfx[SFX_FALL], 0, 1.0, 620, 70, 0.55f, 2.2, TRI, 0.01);
    noise(sfx[SFX_FALL], 0, 1.05, 0.5f, 2.0, 0.30, 0.01, 15);
    // Thắng: rải hợp âm Đô trưởng đi lên rồi ngân.
    sfx[SFX_WIN] = make(1.6);
    const int arp[5] = {72, 76, 79, 84, 88};
    for(int k = 0; k < 5; k++) tone(sfx[SFX_WIN], 0.09 * k, 0.5, midi(arp[k]), midi(arp[k]), 0.32f, 5, SQUARE, 0.005);
    for(int k = 0; k < 3; k++) tone(sfx[SFX_WIN], 0.45, 1.1, midi(arp[k] + 12), midi(arp[k] + 12), 0.16f, 2.5, TRI, 0.02);
    // Tách khối: hai âm lệch nhau rung + lấp lánh.
    sfx[SFX_SPLIT] = make(0.7);
    tone(sfx[SFX_SPLIT], 0, 0.6, 400, 1200, 0.35f, 3, SINE, 0.01);
    tone(sfx[SFX_SPLIT], 0, 0.6, 404, 1180, 0.35f, 3, TRI, 0.01);
    noise(sfx[SFX_SPLIT], 0.05, 0.5, 0.18f, 5, 0.9, 0.9, 16, true);
    // Gộp khối: "bụp".
    sfx[SFX_MERGE] = make(0.25);
    tone(sfx[SFX_MERGE], 0, 0.18, 380, 950, 0.7f, 18, SINE, 0.003);
    tone(sfx[SFX_MERGE], 0.02, 0.15, 760, 1900, 0.2f, 22, TRI);
    // Ô cam vỡ: tiếng rắc.
    sfx[SFX_CRACK] = make(0.45);
    for(int k = 0; k < 5; k++) noise(sfx[SFX_CRACK], 0.035 * k, 0.12, 0.55f - 0.08f * k, 35, 0.8, 0.4, 20 + k, true);
    tone(sfx[SFX_CRACK], 0, 0.2, 150, 80, 0.4f, 18);
    // Bấm nút giao diện.
    sfx[SFX_CLICK] = make(0.05);
    tone(sfx[SFX_CLICK], 0, 0.04, 1500, 1200, 0.5f, 120, TRI);
    // Mở màn: các ô rơi xuống lần lượt.
    sfx[SFX_START] = make(0.9);
    for(int k = 0; k < 6; k++) tone(sfx[SFX_START], 0.11 * k, 0.12, 200 + 25 * k, 120 + 15 * k, 0.35f, 30);
    tone(sfx[SFX_START], 0.62, 0.25, midi(79), midi(79), 0.2f, 8, TRI);
    // Sàn sụp đổ: nhiều tiếng gỗ đổ dồn + rít xuống.
    sfx[SFX_COLLAPSE] = make(1.2);
    Rng r(99);
    for(int k = 0; k < 14; k++)
    {
        double t = 0.06 * k + 0.02 * r.next();
        tone(sfx[SFX_COLLAPSE], t < 0 ? 0 : t, 0.16, 160 + 40 * r.next(), 70, 0.35f, 24);
    }
    noise(sfx[SFX_COLLAPSE], 0, 1.1, 0.25f, 2.5, 0.2, 0.01, 30);

    for(int k = 0; k < SFX_COUNT; k++) normalize(sfx[k], k == SFX_CLICK ? 0.45f : 0.85f);
}

// Nhạc nền chiptune nhẹ, 8 nhịp ~19 giây, lặp liền mạch (các nốt cuối ghi vòng về đầu).
void build_music(Buf &music)
{
    const double beat = 0.6;              // 100 BPM
    const int bars = 8;
    music = Buf(size_t(bars * 4 * beat * SR), 0.0f);
    // Am - F - C - G (hai lượt, lượt hai đổi kiểu rải)
    const int chords[4][3] = {{57, 60, 64}, {53, 57, 60}, {55, 60, 64}, {55, 59, 62}};
    const int roots[4] = {45, 41, 48, 43};
    for(int bar = 0; bar < bars; bar++)
    {
        const int *c = chords[bar % 4];
        double t0 = bar * 4 * beat;
        // Pad: tam giác hai bè lệch nhẹ, attack chậm.
        for(int k = 0; k < 3; k++)
        {
            tone(music, t0, 4 * beat + 0.4, midi(c[k]), midi(c[k]), 0.055f, 0.35, TRI, 0.35, true);
            tone(music, t0, 4 * beat + 0.4, midi(c[k]) * 1.004, midi(c[k]) * 1.004, 0.04f, 0.35, SINE, 0.35, true);
        }
        // Bass: phách 1 và 3.
        for(int k = 0; k < 2; k++)
            tone(music, t0 + 2 * k * beat, 1.2, midi(roots[bar % 4]), midi(roots[bar % 4]), 0.16f, 2.2, SINE, 0.01, true);
        // Rải nốt móc đơn.
        static const int pat1[8] = {0, 1, 2, 1, 0, 1, 2, 3};
        static const int pat2[8] = {2, 1, 0, 1, 3, 2, 1, 2};
        const int *pat = bar < 4 ? pat1 : pat2;
        for(int k = 0; k < 8; k++)
        {
            int idx = pat[k];
            int note = idx < 3 ? c[idx] + 12 : c[0] + 24;
            tone(music, t0 + k * beat / 2, 0.45, midi(note), midi(note), 0.05f, 6, SQUARE, 0.004, true);
        }
        // Hi-hat nhẹ ở phách lệch.
        for(int k = 0; k < 4; k++)
            noise(music, t0 + (k + 0.5) * beat, 0.08, 0.035f, 55, 0.95, 0.95, 40 + bar * 4 + k, true, true);
    }
    // Lọc thông thấp nhẹ cho đỡ chói.
    double y = 0.0;
    for(int pass = 0; pass < 2; pass++)
        for(size_t i = 0; i < music.size(); i++) { y += 0.35 * (music[i] - y); music[i] = float(y); }
    normalize(music, 0.7f);
}

// Thay bằng file của người dùng nếu có: sounds/<tên>.wav|.mp3|.flac
bool load_override(const std::string &name, Buf &out)
{
    static const char *exts[] = {".wav", ".mp3", ".flac"};
    for(int e = 0; e < 3; e++)
    {
        std::string path = "sounds/" + name + exts[e];
        FILE *f = std::fopen(path.c_str(), "rb");
        if(!f) continue;
        std::fclose(f);
        ma_decoder_config cfg = ma_decoder_config_init(ma_format_f32, 1, SR);
        ma_uint64 frames = 0;
        void *data = NULL;
        if(ma_decode_file(path.c_str(), &cfg, &frames, &data) == MA_SUCCESS && frames > 0)
        {
            out.assign((float *)data, (float *)data + frames);
            ma_free(data, NULL);
            return true;
        }
    }
    return false;
}

void callback(ma_device *, void *output, const void *, ma_uint32 frames)
{
    float *o = (float *)output;
    std::memset(o, 0, sizeof(float) * 2 * frames);
    std::lock_guard<std::mutex> lock(mtx);
    if(!music.empty() && vol_music > 0.0f)
        for(ma_uint32 i = 0; i < frames; i++)
        {
            if(music_fade < 1.0f) music_fade += 1.0f / (2.5f * SR);
            float v = music[music_pos] * vol_music * music_fade;
            if(++music_pos >= music.size()) music_pos = 0;
            o[2 * i] += v;
            o[2 * i + 1] += v;
        }
    for(size_t k = 0; k < voices.size(); )
    {
        Voice &v = voices[k];
        const Buf &b = *v.buf;
        float g = v.gain * vol_sfx;
        for(ma_uint32 i = 0; i < frames; i++)
        {
            size_t idx = size_t(v.pos);
            if(idx + 1 >= b.size()) { v.pos = double(b.size()); break; }
            double fr = v.pos - double(idx);
            float s = float(b[idx] * (1.0 - fr) + b[idx + 1] * fr) * g;
            o[2 * i] += s;
            o[2 * i + 1] += s;
            v.pos += v.rate;
        }
        if(size_t(v.pos) + 1 >= b.size()) { voices[k] = voices.back(); voices.pop_back(); }
        else k++;
    }
    for(ma_uint32 i = 0; i < 2 * frames; i++)
        o[i] = std::tanh(o[i]);   // nén mềm, tránh vỡ tiếng khi nhiều âm chồng lên nhau
}

} // namespace

void audio_init(bool enabled)
{
    if(!enabled) { status_text = "muted (--mute)"; return; }
    build_sfx();
    int custom = 0;
    for(int k = 0; k < SFX_COUNT; k++)
        custom += load_override(SFX_NAMES[k], sfx[k]);
    Buf custom_music;
    bool has_custom_music = load_override("music", custom_music);
    custom += has_custom_music;

    ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format = ma_format_f32;
    cfg.playback.channels = 2;
    cfg.sampleRate = SR;
    cfg.dataCallback = callback;
    if(ma_device_init(NULL, &cfg, &device) != MA_SUCCESS)
    {
        status_text = "no audio device";
        return;
    }
    if(ma_device_start(&device) != MA_SUCCESS)
    {
        ma_device_uninit(&device);
        status_text = "audio device failed to start";
        return;
    }
    device_ok = true;
    // Nhạc nền tổng hợp mất ~1 s: làm ở luồng nền để cửa sổ mở ngay, xong thì gắn vào bộ trộn.
    music_thread = std::thread([has_custom_music, custom_music]() {
        Buf m = custom_music;
        if(!has_custom_music) build_music(m);
        std::lock_guard<std::mutex> lock(mtx);
        music.swap(m);
        music_pos = 0;
        music_fade = 0.0f;
    });
    char buf[64];
    if(custom) std::snprintf(buf, sizeof(buf), "procedural + %d custom file(s)", custom);
    else std::snprintf(buf, sizeof(buf), "procedural");
    status_text = buf;
}

void audio_shutdown()
{
    if(music_thread.joinable()) music_thread.join();
    if(device_ok) ma_device_uninit(&device);
    device_ok = false;
}

void audio_play(int id, float gain, float rate)
{
    if(!device_ok || id < 0 || id >= SFX_COUNT || sfx[id].empty()) return;
    std::lock_guard<std::mutex> lock(mtx);
    if(voices.size() >= 32) voices.erase(voices.begin());
    Voice v = {&sfx[id], 0.0, gain, rate};
    voices.push_back(v);
}

bool audio_available() { return device_ok; }

void audio_set_volumes(float m, float s)
{
    std::lock_guard<std::mutex> lock(mtx);
    vol_music = m < 0 ? 0 : (m > 1 ? 1 : m);
    vol_sfx = s < 0 ? 0 : (s > 1 ? 1 : s);
}

const char *audio_status() { return status_text.c_str(); }
