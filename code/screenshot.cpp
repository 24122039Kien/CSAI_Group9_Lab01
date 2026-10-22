#include "headers.h"
#include "screenshot.h"
#include "noedit.h"

#include <cstdint>

// Bộ ghi PNG tối giản: dữ liệu RGB, nén "stored" (không nén) trong zlib — đủ cho ảnh chụp màn hình,
// không cần thư viện ngoài.

static uint32_t crc_table[256];

static void init_crc()
{
    for(uint32_t n=0; n<256; n++)
    {
        uint32_t c = n;
        for(int k=0; k<8; k++)
            c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        crc_table[n] = c;
    }
}

static uint32_t crc(const unsigned char *buf, size_t len, uint32_t c = 0xFFFFFFFFu)
{
    for(size_t i=0; i<len; i++)
        c = crc_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
    return c;
}

static void put32(vector<unsigned char> &v, uint32_t x)
{
    v.push_back(x >> 24); v.push_back(x >> 16); v.push_back(x >> 8); v.push_back(x);
}

static void chunk(FILE *out, const char *type, const vector<unsigned char> &data)
{
    vector<unsigned char> buf;
    put32(buf, (uint32_t)data.size());
    buf.insert(buf.end(), type, type + 4);
    buf.insert(buf.end(), data.begin(), data.end());
    uint32_t c = crc(&buf[4], buf.size() - 4) ^ 0xFFFFFFFFu;
    put32(buf, c);
    fwrite(&buf[0], 1, buf.size(), out);
}

bool save_framebuffer_png(const string &path, int width, int height)
{
    init_crc();
    vector<unsigned char> pixels(width * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);

    // Dòng ảnh PNG đi từ trên xuống, mỗi dòng có 1 byte filter (0).
    vector<unsigned char> raw;
    raw.reserve((width * 3 + 1) * height);
    for(int y=height-1; y>=0; y--)
    {
        raw.push_back(0);
        raw.insert(raw.end(), pixels.begin() + y*width*3, pixels.begin() + (y+1)*width*3);
    }

    vector<unsigned char> z;
    z.push_back(0x78); z.push_back(0x01);
    uint32_t a = 1, b = 0;
    for(size_t pos=0; pos<raw.size(); )
    {
        size_t len = min<size_t>(65535, raw.size() - pos);
        z.push_back(pos + len == raw.size() ? 1 : 0);
        z.push_back(len & 0xFF); z.push_back(len >> 8);
        z.push_back(~len & 0xFF); z.push_back((~len >> 8) & 0xFF);
        z.insert(z.end(), raw.begin() + pos, raw.begin() + pos + len);
        for(size_t i=pos; i<pos+len; i++)
        {
            a = (a + raw[i]) % 65521;
            b = (b + a) % 65521;
        }
        pos += len;
    }
    put32(z, (b << 16) | a);

    FILE *out = utf8_fopen(path, "wb");
    if(!out)
        return false;
    const unsigned char sig[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    fwrite(sig, 1, 8, out);
    vector<unsigned char> ihdr;
    put32(ihdr, width);
    put32(ihdr, height);
    ihdr.push_back(8); ihdr.push_back(2); ihdr.push_back(0); ihdr.push_back(0); ihdr.push_back(0);
    chunk(out, "IHDR", ihdr);
    chunk(out, "IDAT", z);
    chunk(out, "IEND", vector<unsigned char>());
    return fclose(out) == 0;
}
