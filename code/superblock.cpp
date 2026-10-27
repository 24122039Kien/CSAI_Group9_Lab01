#include "headers.h"
#include "superblock.h"
#include "tile.h"
#include "game.h"
#include "render.h"
#include "effects.h"

int X=0, Y=1, Z=2;
int U=1, D=2, R=3, L=4, NO=0;
// Tốc độ lăn theo thời gian thật (độ/giây). Bản gốc cộng 5 độ mỗi frame nên FPS thấp thì lăn chậm.
float roll_speed = 450.0f;

enum { END_NONE = 0, END_SINK = 1, END_FALL = 2 };

SuperBlock::SuperBlock(){}

SuperBlock::SuperBlock(glm::vec3 pos, glm::vec3 dim, Color c, int a)
{
    this->initialize(pos, dim, c, a);
}

glm::vec3 SuperBlock::get_real_pos()
{
    if(is_split)
    {
        float l = tile_side_length;
        return glm::vec3(cube_ix[active_cube] * l + l/2.0f, cube_iy[active_cube] * l + l/2.0f, l/2.0f);
    }
    return real_pos;
}

void SuperBlock::initialize(glm::vec3 pos, glm::vec3 dim, Color c, int a)
{
    this->pos = pos;
    this->dim = dim;
    this->color = c;
    real_pos = pos;
    along = a;
    rot_dir = 0;
    last_dir = R;
    rem_rotation = 0.0f;
    done_rotation = 0.0f;
    end_anim = END_NONE;
    end_start = 0.0;
    fall_dir = glm::vec2(1, 0);
    pair<int, int> pp = stage.get_initial_index();
    ix=pp.first;
    iy=pp.second;
    model = glm::mat4(1.0f);
    is_split = false;
    active_cube = 0;
}

void SuperBlock::update(float dt)
{
    if(rot_dir!=NO)
    {
        done_rotation = min(90.0f, done_rotation + roll_speed*dt);
        rem_rotation = 90.0f - done_rotation;
    }
    if(rot_dir!=NO && rem_rotation<=0.0f)
    {
        if(is_split)
        {
            if(rot_dir == R) cube_ix[active_cube]++;
            else if(rot_dir == L) cube_ix[active_cube]--;
            else if(rot_dir == U) cube_iy[active_cube]++;
            else if(rot_dir == D) cube_iy[active_cube]--;

            float l = tile_side_length;
            cube_pos[active_cube] = glm::vec3(cube_ix[active_cube] * l + l/2.0f,
                                              cube_iy[active_cube] * l + l/2.0f,
                                              l/2.0f);

            rot_dir = NO;
            game.check_after_step();
            land_dust();
            return;
        }

        float x = tile_side_length;
        if(along==X)
        {
            if(rot_dir==L)
            {
                along = Z;
                pos = glm::vec3(-3*x/2, 0, x/2) + pos;
                ix--;
            }
            else if(rot_dir==R)
            {
                along = Z;
                pos = glm::vec3(3*x/2, 0, x/2) + pos;
                ix+=2;
            }
            else if(rot_dir==U)
            {
                along = X;
                pos = glm::vec3(0, x, 0) + pos;
                iy++;
            }
            else if(rot_dir==D)
            {
                along = X;
                pos = glm::vec3(0, -x, 0) + pos;
                iy--;
            }
        }
        else if(along==Y)
        {
            if(rot_dir==U)
            {
                along = Z;
                pos = glm::vec3(0, 3*x/2, x/2) + pos;
                iy+=2;
            }
            else if(rot_dir==D)
            {
                along = Z;
                pos = glm::vec3(0, -3*x/2, x/2) + pos;
                iy--;
            }
            if(rot_dir==R)
            {
                along = Y;
                pos = glm::vec3(x, 0, 0) + pos;
                ix++;
            }
            else if(rot_dir==L)
            {
                along = Y;
                pos = glm::vec3(-x, 0, 0) + pos;
                ix--;
            }
        }
        else if(along==Z)
        {
            if(rot_dir==L)
            {
                along = X;
                pos = glm::vec3(-3*x/2, 0, -x/2) + pos;
                ix-=2;
            }
            else if(rot_dir==R)
            {
                along = X;
                pos = glm::vec3(3*x/2, 0, -x/2) + pos;
                ix++;
            }
            else if(rot_dir==U)
            {
                along = Y;
                pos = glm::vec3(0, 3*x/2, -x/2) + pos;
                iy++;
            }
            else if(rot_dir==D)
            {
                along = Y;
                pos = glm::vec3(0, -3*x/2, -x/2) + pos;
                iy-=2;
            }
        }
        rot_dir = NO;
        game.check_after_step();
        land_dust();
    }
}

// Bụi bay ra quanh chân khối mỗi lần lăn xong (chỉ hình ảnh).
void SuperBlock::land_dust()
{
    float l = tile_side_length;
    if(is_split)
    {
        glm::vec3 c(cube_ix[active_cube]*l + l/2.0f, cube_iy[active_cube]*l + l/2.0f, 0.0f);
        effects.dust(c, glm::vec2(l/2.0f, l/2.0f));
        return;
    }
    glm::vec3 c(ix*l + l/2, iy*l + l/2, 0.0f);
    glm::vec2 half(l/2, l/2);
    if(along==X) { c.x += l/2; half.x = l; }
    else if(along==Y) { c.y += l/2; half.y = l; }
    effects.dust(c, half);
}

void SuperBlock::draw(glm::mat4 M)
{
    if(is_split)
    {
        float l = tile_side_length;
        float dr = 90.0f * ease_in_out(done_rotation / 90.0f);
        glm::vec3 csize(l, l, l);
        glm::vec3 default_col = (color.r > 0.01f || color.g > 0.01f || color.b > 0.01f)
                              ? glm::vec3(color.r, color.g, color.b)
                              : col(BLOCK_COLOR);

        for(int k=0; k<2; k++)
        {
            bool is_active = (k == active_cube);
            // Khối được chọn có màu vàng kim sáng (golden amber) nổi bật,
            // khối chưa được chọn có màu đỏ gạch mặc định của khối 1x1x2 ban đầu.
            glm::vec3 c_col = is_active ? glm::vec3(1.0f, 0.85f, 0.20f) : default_col;
            BoxStyle s(c_col, PAT_PLAIN, is_active ? 0.35f : 0.04f);

            glm::vec3 P0(cube_ix[k] * l, cube_iy[k] * l, 0.0f);
            if(is_active && rot_dir != NO)
            {
                glm::vec3 pivot(0.0f);
                glm::vec3 axis(0.0f);
                float angle = dr;
                if(rot_dir == R)
                {
                    pivot = glm::vec3(P0.x + l, P0.y + l/2.0f, 0.0f);
                    axis = glm::vec3(0, 1, 0);
                    angle = dr;
                }
                else if(rot_dir == L)
                {
                    pivot = glm::vec3(P0.x, P0.y + l/2.0f, 0.0f);
                    axis = glm::vec3(0, 1, 0);
                    angle = -dr;
                }
                else if(rot_dir == U)
                {
                    pivot = glm::vec3(P0.x + l/2.0f, P0.y + l, 0.0f);
                    axis = glm::vec3(1, 0, 0);
                    angle = -dr;
                }
                else if(rot_dir == D)
                {
                    pivot = glm::vec3(P0.x + l/2.0f, P0.y, 0.0f);
                    axis = glm::vec3(1, 0, 0);
                    angle = dr;
                }

                glm::mat4 T1 = glm::translate(pivot);
                glm::mat4 Rmat = glm::rotate(glm::mat4(1.0f), float(angle * M_PI / 180.0f), axis);
                glm::mat4 T2 = glm::translate(-pivot);
                glm::mat4 box = T1 * Rmat * T2 * glm::translate(P0) * glm::scale(csize);
                renderer.draw_box(box, csize, s);
            }
            else
            {
                renderer.draw_aabb(P0, csize, s);
            }
        }
        return;
    }

    glm::mat4 translate = glm::translate (pos);
    glm::mat4 rev;
    M = M * translate;

    glm::mat4 rotate(1.0f);
    if(along==Y)
        rotate = rotate * glm::rotate(glm::mat4(1.0f), float(-M_PI/2), glm::vec3(0,0,1));
    else if(along==Z)
        rotate = rotate * glm::rotate(glm::mat4(1.0f), float(-M_PI/2), glm::vec3(0,1,0));

    M = M * rotate;

    // Góc hiển thị có easing (tăng tốc rồi giảm tốc); luật chỉ dùng done_rotation.
    float dr = 90.0f * ease_in_out(done_rotation / 90.0f);
    float x = tile_side_length;
    real_pos = pos;
    real_pos.z = 0;

    if(along==X)
    {
        if(rot_dir==R)
        {
            translate = glm::translate(glm::vec3(x, 0, -x/2));
            rotate = glm::rotate(glm::mat4(1.0f), float(M_PI*dr/180.0f), glm::vec3(0,1,0));
            rev = glm::translate(glm::vec3(-x, 0, x/2));
            M = M * translate * rotate * rev;
            real_pos.x += tile_side_length * (1-cos(dr*M_PI/180) + 0.5*sin(dr*M_PI/180));
        }
        else if(rot_dir==L)
        {
            translate = glm::translate(glm::vec3(-x, 0, -x/2));
            rotate = glm::rotate(glm::mat4(1.0f), float(-M_PI*dr/180.0f), glm::vec3(0,1,0));
            rev = glm::translate(glm::vec3(x, 0, x/2));
            M = M * translate * rotate * rev;
            real_pos.x -= tile_side_length * (1-cos(dr*M_PI/180)+ 0.5*sin(dr*M_PI/180));
        }
        else if(rot_dir==U)
        {
            translate = glm::translate(glm::vec3(0, x/2, -x/2));
            rotate = glm::rotate(glm::mat4(1.0f), float(M_PI*dr/180.0f), glm::vec3(-1,0,0));
            rev = glm::translate(glm::vec3(0, -x/2, x/2));
            M = M * translate * rotate * rev;
            real_pos.y += 0.5*tile_side_length * (1-cos(dr*M_PI/180)+ sin(dr*M_PI/180));
        }
        else if(rot_dir==D)
        {
            translate = glm::translate(glm::vec3(0, -x/2, -x/2));
            rotate = glm::rotate(glm::mat4(1.0f), float(-M_PI*dr/180.0f), glm::vec3(-1,0,0));
            rev = glm::translate(glm::vec3(0, x/2, x/2));
            M = M * translate * rotate * rev;
            real_pos.y -= 0.5*tile_side_length * (1-cos(dr*M_PI/180)+ sin(dr*M_PI/180));
        }
    }
    else if(along==Y)
    {
        if(rot_dir==R)
        {
            translate = glm::translate(glm::vec3(0, x/2, -x/2));
            rotate = glm::rotate(glm::mat4(1.0f), float(M_PI*dr/180.0f), glm::vec3(-1,0,0));
            rev = glm::translate(glm::vec3(0, -x/2, x/2));
            M = M * translate * rotate * rev;
            real_pos.x += 0.5*tile_side_length * (1-cos(dr*M_PI/180)+ sin(dr*M_PI/180));
        }
        else if(rot_dir==L)
        {
            translate = glm::translate(glm::vec3(0, -x/2, -x/2));
            rotate = glm::rotate(glm::mat4(1.0f), float(M_PI*dr/180.0f), glm::vec3(1,0,0));
            rev = glm::translate(glm::vec3(0, x/2, x/2));
            M = M * translate * rotate * rev;
            real_pos.x -= 0.5*tile_side_length * (1-cos(dr*M_PI/180)+ sin(dr*M_PI/180));
        }
        else if(rot_dir==U)
        {
            translate = glm::translate(glm::vec3(-x, 0, -x/2));
            rotate = glm::rotate(glm::mat4(1.0f), float(M_PI*dr/180.0f), glm::vec3(0,-1,0));
            rev = glm::translate(glm::vec3(x, 0, x/2));
            M = M * translate * rotate * rev;
            real_pos.y += tile_side_length * (1-cos(dr*M_PI/180)+ 0.5*sin(dr*M_PI/180));
        }
        else if(rot_dir==D)
        {
            translate = glm::translate(glm::vec3(x, 0, -x/2));
            rotate = glm::rotate(glm::mat4(1.0f), float(M_PI*dr/180.0f), glm::vec3(0,1,0));
            rev = glm::translate(glm::vec3(-x, 0, x/2));
            M = M * translate * rotate * rev;
            real_pos.y -= tile_side_length * (1-cos(dr*M_PI/180)+ 0.5*sin(dr*M_PI/180));
        }
    }
    else if(along==Z)
    {
        if(rot_dir==R)
        {
            //cout << pos.x << " " << pos.y << " " << pos.z << endl;
            translate = glm::translate(glm::vec3(-x, 0, -x/2));
            rotate = glm::rotate(glm::mat4(1.0f), float(M_PI*dr/180.0f), glm::vec3(0,1,0));
            rev = glm::translate(glm::vec3(x, 0, x/2));
            M = M * translate * rotate * rev;
            real_pos.x += tile_side_length * (1-cos(dr*M_PI/180)+ 0.5*sin(dr*M_PI/180));
        }
        else if(rot_dir==L)
        {
            translate = glm::translate(glm::vec3(-x, 0, x/2));
            rotate = glm::rotate(glm::mat4(1.0f), float(-M_PI*dr/180.0f), glm::vec3(0,1,0));
            rev = glm::translate(glm::vec3(x, 0, -x/2));
            M = M * translate * rotate * rev;
            real_pos.x -= tile_side_length * (1-cos(dr*M_PI/180)+ 0.5*sin(dr*M_PI/180));
        }
        else if(rot_dir==U)
        {
            translate = glm::translate(glm::vec3(-x, x/2, 0));
            rotate = glm::rotate(glm::mat4(1.0f), float(M_PI*dr/180.0f), glm::vec3(0,0,1));
            rev = glm::translate(glm::vec3(x, -x/2, 0));
            M = M * translate * rotate * rev;
            real_pos.y += tile_side_length * (1-cos(dr*M_PI/180)+ 0.5*sin(dr*M_PI/180));
        }
        else if(rot_dir==D)
        {
            translate = glm::translate(glm::vec3(-x, -x/2, 0));
            rotate = glm::rotate(glm::mat4(1.0f), float(M_PI*dr/180.0f), glm::vec3(0,0,-1));
            rev = glm::translate(glm::vec3(x, x/2, 0));
            M = M * translate * rotate * rev;
            real_pos.y -= tile_side_length * (1-cos(dr*M_PI/180)+ 0.5*sin(dr*M_PI/180));
        }
    }


    // Hộp đơn vị [0,1]^3 -> hộp tâm tại gốc, kích thước dim.
    glm::mat4 box = M * glm::translate(-dim/2.0f) * glm::scale(dim);

    // Hiệu ứng: rơi xuống khi mở màn, chìm vào lỗ khi thắng, lật và rơi khi thua.
    double now = glfwGetTime();
    float dz = effects.block_drop(now);
    glm::mat4 fx(1.0f);
    float t = float(now - end_start);
    if(end_anim == END_SINK)
        dz -= 2.6f * ease_in_out(t / 1.1f);
    else if(end_anim == END_FALL)
    {
        glm::vec3 c = glm::vec3(box * glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        glm::vec3 drift = glm::vec3(fall_dir * min(t, 1.5f) * 0.9f, 0.0f);
        glm::vec3 axis = glm::cross(glm::vec3(0, 0, 1), glm::vec3(fall_dir, 0.0f));
        float angle = min(t * 3.0f, 1.8f);
        fx = glm::translate(c + drift) * glm::rotate(angle, axis) * glm::translate(-c);
        dz -= 7.0f * t * t;
    }
    model = glm::translate(glm::vec3(0, 0, dz)) * fx * box;

    BoxStyle s(glm::vec3(color.r, color.g, color.b), PAT_PLAIN, 0.04f);
    renderer.draw_box(model, dim, s);
}

// Bóng mềm dưới khối (vẽ trong lượt trong suốt, sau khi đã vẽ sàn).
void SuperBlock::draw_shadow()
{
    if(end_anim == END_FALL)
        return;
    if(is_split)
    {
        float l = tile_side_length;
        float pad = 0.15f;
        BoxStyle s(glm::vec3(0.0f), PAT_SHADOW, 0.0f, 0.5f);
        for(int k=0; k<2; k++)
        {
            float x = cube_ix[k] * l;
            float y = cube_iy[k] * l;
            float sx = l;
            float sy = l;
            if(k == active_cube && rot_dir != NO)
            {
                float prog = done_rotation / 90.0f;
                if(rot_dir == R) { sx = l * (1.0f + prog); }
                else if(rot_dir == L) { x -= l * prog; sx = l * (1.0f + prog); }
                else if(rot_dir == U) { sy = l * (1.0f + prog); }
                else if(rot_dir == D) { y -= l * prog; sy = l * (1.0f + prog); }
            }
            renderer.draw_aabb(glm::vec3(x - pad, y - pad, 0.004f),
                               glm::vec3(sx + 2*pad, sy + 2*pad, 0.001f), s);
        }
        return;
    }
    glm::vec3 lo(1e9f), hi(-1e9f);
    for(int k=0; k<8; k++)
    {
        glm::vec4 p = model * glm::vec4(float(k&1), float((k>>1)&1), float((k>>2)&1), 1.0f);
        lo = glm::min(lo, glm::vec3(p));
        hi = glm::max(hi, glm::vec3(p));
    }
    float height = max(0.0f, lo.z);
    float alpha = 0.6f * clamp01(1.0f - height / 6.0f);
    if(end_anim == END_SINK)
        alpha *= clamp01(1.0f - float(glfwGetTime() - end_start) / 0.6f);
    if(alpha <= 0.01f)
        return;
    float pad = 0.15f + 0.05f * height;
    BoxStyle s(glm::vec3(0.0f), PAT_SHADOW, 0.0f, alpha);
    renderer.draw_aabb(glm::vec3(lo.x - pad, lo.y - pad, 0.004f),
                       glm::vec3(hi.x - lo.x + 2*pad, hi.y - lo.y + 2*pad, 0.001f), s);
}

void SuperBlock::start_sink(double now)
{
    end_anim = END_SINK;
    end_start = now;
}

void SuperBlock::start_fall(double now)
{
    // Lật về phía không có sàn (nếu một nửa khối còn trên sàn), ngược lại theo hướng vừa lăn.
    glm::vec2 d(0.0f);
    if(last_dir==R) d = glm::vec2(1, 0);
    else if(last_dir==L) d = glm::vec2(-1, 0);
    else if(last_dir==U) d = glm::vec2(0, 1);
    else if(last_dir==D) d = glm::vec2(0, -1);
    if(along==X)
    {
        bool a = stage.has_tile(ix, iy), b = stage.has_tile(ix+1, iy);
        if(a && !b) d = glm::vec2(1, 0);
        else if(!a && b) d = glm::vec2(-1, 0);
    }
    else if(along==Y)
    {
        bool a = stage.has_tile(ix, iy), b = stage.has_tile(ix, iy+1);
        if(a && !b) d = glm::vec2(0, 1);
        else if(!a && b) d = glm::vec2(0, -1);
    }
    fall_dir = d;
    end_anim = END_FALL;
    end_start = now;
}

bool SuperBlock::is_moving()
{
    return rot_dir != NO;
}

void SuperBlock::move(int m)
{
    if(rot_dir!=NO)return;

    if(is_split)
    {
        int dir = NO;
        int dx = 0, dy = 0;
        if(m==GLFW_KEY_RIGHT) { dir = R; dx = 1; }
        else if(m==GLFW_KEY_LEFT) { dir = L; dx = -1; }
        else if(m==GLFW_KEY_UP) { dir = U; dy = 1; }
        else if(m==GLFW_KEY_DOWN) { dir = D; dy = -1; }

        if(dir != NO)
        {
            int nx = cube_ix[active_cube] + dx;
            int ny = cube_iy[active_cube] + dy;
            int other = 1 - active_cube;
            // Hai khối không được đè lên cùng 1 ô
            if(nx == cube_ix[other] && ny == cube_iy[other])
                return;

            rot_dir = dir;
            rem_rotation = 90.0f;
            done_rotation = 0.0f;
            last_dir = rot_dir;
        }
        return;
    }

    if(m==GLFW_KEY_UP)
    {
        rot_dir = U;
        rem_rotation = 90.0f;
        done_rotation = 0.0f;
    }
    else if(m==GLFW_KEY_DOWN)
    {
        rot_dir = D;
        rem_rotation = 90.0f;
        done_rotation = 0.0f;
    }
    else if(m==GLFW_KEY_LEFT)
    {
        rot_dir = L;
        rem_rotation = 90.0f;
        done_rotation = 0.0f;
    }
    else if(m==GLFW_KEY_RIGHT)
    {
        rot_dir = R;
        rem_rotation = 90.0f;
        done_rotation = 0.0f;
    }
    if(rot_dir!=NO)
        last_dir = rot_dir;
}

void SuperBlock::split(int x1, int y1, int x2, int y2)
{
    is_split = true;
    active_cube = 0;
    cube_ix[0] = x1; cube_iy[0] = y1;
    cube_ix[1] = x2; cube_iy[1] = y2;
    float l = tile_side_length;
    cube_pos[0] = glm::vec3(x1 * l + l/2.0f, y1 * l + l/2.0f, l/2.0f);
    cube_pos[1] = glm::vec3(x2 * l + l/2.0f, y2 * l + l/2.0f, l/2.0f);
}

void SuperBlock::switch_active_cube()
{
    if(is_split && rot_dir == NO)
    {
        active_cube = 1 - active_cube;
    }
}

bool SuperBlock::check_recombine()
{
    if(!is_split) return false;
    int dist = abs(cube_ix[0] - cube_ix[1]) + abs(cube_iy[0] - cube_iy[1]);
    if(dist == 1)
    {
        is_split = false;
        float l = tile_side_length;
        if(abs(cube_ix[0] - cube_ix[1]) == 1)
        {
            along = X;
            ix = min(cube_ix[0], cube_ix[1]);
            iy = cube_iy[0];
            pos = glm::vec3(ix * l + l, iy * l + l/2.0f, l/2.0f);
        }
        else
        {
            along = Y;
            ix = cube_ix[0];
            iy = min(cube_iy[0], cube_iy[1]);
            pos = glm::vec3(ix * l + l/2.0f, iy * l + l, l/2.0f);
        }
        real_pos = pos;
        return true;
    }
    return false;
}

SuperBlock::~SuperBlock()
{
}
