#include "headers.h"
#include "colors.h"
#include "render.h"
#include "effects.h"

Effects effects;

static const float INTRO_SPREAD = 0.55f;    // ô xa nhất bắt đầu rơi sau 0.55 s
static const float TILE_FALL = 0.40f;
static const float BLOCK_START = 0.75f;
static const float BLOCK_FALL = 0.45f;
static const size_t MAX_PARTICLES = 600;

static float frand(float a, float b) { return a + (b - a) * (rand() / (float)RAND_MAX); }

Effects::Effects()
{
    intro_start = -100.0;
    enabled = true;
}

void Effects::start_intro(double now)
{
    intro_start = now;
    particles.clear();
}

bool Effects::intro_active(double now)
{
    return enabled && now - intro_start < BLOCK_START + BLOCK_FALL;
}

float Effects::tile_drop(int i, int j, int n, int m, double now)
{
    if(!enabled) return 0.0f;
    float delay = INTRO_SPREAD * (i + j) / float(max(1, n + m - 2));
    float t = float(now - intro_start - delay) / TILE_FALL;
    return (1.0f - ease_out_cubic(t)) * 12.0f;
}

float Effects::block_drop(double now)
{
    if(!enabled) return 0.0f;
    float t = float(now - intro_start - BLOCK_START) / BLOCK_FALL;
    return (1.0f - ease_out_bounce(t)) * 8.0f;
}

void Effects::dust(glm::vec3 center, glm::vec2 half)
{
    if(!enabled) return;
    for(int k=0; k<18 && particles.size()<MAX_PARTICLES; k++)
    {
        // Hạt xuất hiện quanh chu vi chân khối, bay ra ngoài và lên nhẹ.
        float side = frand(0.0f, 4.0f);
        glm::vec2 p;
        if(side < 1.0f)      p = glm::vec2(frand(-1,1)*half.x, -half.y);
        else if(side < 2.0f) p = glm::vec2(frand(-1,1)*half.x,  half.y);
        else if(side < 3.0f) p = glm::vec2(-half.x, frand(-1,1)*half.y);
        else                 p = glm::vec2( half.x, frand(-1,1)*half.y);
        glm::vec2 out = glm::normalize(p + glm::vec2(frand(-0.1f,0.1f), frand(-0.1f,0.1f)));
        Particle q;
        q.pos = center + glm::vec3(p, 0.03f);
        q.vel = glm::vec3(out * frand(0.4f, 1.2f), frand(0.3f, 1.0f));
        q.color = col(DUST_COLOR);
        q.max_life = q.life = frand(0.35f, 0.6f);
        q.size = frand(0.04f, 0.08f);
        q.glow = 0.1f;
        particles.push_back(q);
    }
}

void Effects::burst(glm::vec3 p, glm::vec3 color, int count, float speed)
{
    if(!enabled) return;
    for(int k=0; k<count && particles.size()<MAX_PARTICLES; k++)
    {
        float a = frand(0.0f, 2.0f*(float)M_PI);
        float up = frand(0.6f, 1.6f);
        Particle q;
        q.pos = p + glm::vec3(frand(-0.3f,0.3f), frand(-0.3f,0.3f), 0.0f);
        q.vel = glm::vec3(cos(a)*speed*frand(0.2f,1.0f), sin(a)*speed*frand(0.2f,1.0f), up*speed);
        q.color = glm::mix(color, glm::vec3(1.0f), frand(0.0f, 0.5f));
        q.max_life = q.life = frand(0.8f, 1.6f);
        q.size = frand(0.05f, 0.11f);
        q.glow = 1.2f;
        particles.push_back(q);
    }
}

void Effects::update(float dt)
{
    for(size_t k=0; k<particles.size(); )
    {
        Particle &q = particles[k];
        q.life -= dt;
        if(q.life <= 0.0f)
        {
            particles[k] = particles.back();
            particles.pop_back();
            continue;
        }
        q.vel.z -= 3.5f * dt;
        q.vel *= (1.0f - 1.5f*dt);
        q.pos += q.vel * dt;
        k++;
    }
}

void Effects::draw()
{
    for(size_t k=0; k<particles.size(); k++)
    {
        const Particle &q = particles[k];
        float f = q.life / q.max_life;
        float s = q.size * (0.4f + 0.6f*f);
        BoxStyle st(q.color, PAT_PLAIN, q.glow, f);
        st.unlit = true;
        renderer.draw_aabb(q.pos - glm::vec3(s*0.5f), glm::vec3(s), st);
    }
}

void Effects::clear()
{
    particles.clear();
}
