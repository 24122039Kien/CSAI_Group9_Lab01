#ifndef EFFECTS
#define EFFECTS

// Hiệu ứng thuần hình ảnh (mở màn, hạt bụi, pháo sáng). Không đọc/ghi trạng thái luật.

inline float clamp01(float t) { return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t); }
inline float ease_out_cubic(float t) { t = clamp01(t); float u = 1.0f - t; return 1.0f - u*u*u; }
inline float ease_in_out(float t) { t = clamp01(t); return t*t*(3.0f - 2.0f*t); }
inline float ease_out_bounce(float t)
{
    t = clamp01(t);
    const float n = 7.5625f, d = 2.75f;
    if(t < 1.0f/d) return n*t*t;
    if(t < 2.0f/d) { t -= 1.5f/d; return n*t*t + 0.75f; }
    if(t < 2.5f/d) { t -= 2.25f/d; return n*t*t + 0.9375f; }
    t -= 2.625f/d; return n*t*t + 0.984375f;
}

struct Particle
{
    glm::vec3 pos, vel, color;
    float life, max_life, size, glow;
};

class Effects
{
    private:
        vector<Particle> particles;
        double intro_start;
    public:
        bool enabled;               // tắt được trong menu (máy yếu)
        Effects();
        void start_intro(double now);
        bool intro_active(double now);
        float tile_drop(int i, int j, int n, int m, double now);   // độ cao cộng thêm của ô khi mở màn
        float block_drop(double now);                              // độ cao cộng thêm của khối khi mở màn
        void dust(glm::vec3 center, glm::vec2 half);               // bụi khi khối chạm sàn
        void burst(glm::vec3 p, glm::vec3 color, int count, float speed);
        void update(float dt);
        void draw();
        void clear();
        size_t count() { return particles.size(); }
};

extern Effects effects;

#endif
