#ifndef AUDIO
#define AUDIO

// Âm thanh: hiệu ứng + nhạc nền tổng hợp bằng code lúc khởi động (không cần file đi kèm).
// Muốn thay âm thanh: đặt sounds/<tên>.wav|.mp3|.flac cạnh file chạy (tên: xem SFX_NAMES trong
// audio.cpp, nhạc nền là sounds/music.*). Chỉ là phần trình bày, không đụng tới luật chơi.

enum Sfx
{
    SFX_ROLL = 0, SFX_STAND, SFX_SWITCH, SFX_HEAVY, SFX_BRIDGE_ON, SFX_BRIDGE_OFF, SFX_FALL,
    SFX_WIN, SFX_SPLIT, SFX_MERGE, SFX_CRACK, SFX_CLICK, SFX_START, SFX_COLLAPSE, SFX_COUNT
};

void audio_init(bool enabled);              // mở thiết bị âm thanh; lỗi thì game vẫn chạy (không tiếng)
void audio_shutdown();
void audio_play(int sfx, float gain = 1.0f, float rate = 1.0f);
bool audio_available();
void audio_set_volumes(float music, float sfx);   // 0..1
const char *audio_status();                 // mô tả ngắn cho menu ("procedural", "custom: 3 file", lỗi...)

#endif
