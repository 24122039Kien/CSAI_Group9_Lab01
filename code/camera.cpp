#include "headers.h"
#include "camera.h"
#include "controls.h"
#include "superblock.h"
#include "tile.h"
#include "hud.h"

Camera camera;
float camera_move_unit = 0.1f;
float camera_dir_change_unit = 0.02f;
int helicopter=GLFW_KEY_X, vertical=GLFW_KEY_V, tower=GLFW_KEY_B, follow=GLFW_KEY_C, block_view=GLFW_KEY_Z,
    mouse_view=GLFW_KEY_M, mouse_view_cs, drag_view=GLFW_KEY_N;

static const glm::vec3 UP(0.0f, 0.0f, 1.0f);
static const float MIN_PITCH = 8.0f, MAX_PITCH = 88.0f;
static const float MIN_DIST = 2.5f, MAX_DIST = 90.0f;

// Khung bao các ô thật của màn (không tính lề trống của lưới), tính lại mỗi khi nạp màn.
glm::vec3 board_center(0.0f);
float board_radius = 5.0f;

static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

static glm::vec3 orbit_offset(float yaw, float pitch, float dist)
{
    float y = glm::radians(yaw), p = glm::radians(pitch);
    return dist * glm::vec3(cos(p) * cos(y), cos(p) * sin(y), sin(p));
}

static void compute_board_bounds()
{
    float l = tile_side_length;
    glm::vec2 lo(1e9f), hi(-1e9f);
    for(int i=0; i<stage.n; i++)
        for(int j=0; j<stage.m; j++)
            if(stage.tiles[i][j] || (i == stage.tx && j == stage.ty))
            {
                lo = glm::min(lo, glm::vec2(i * l, j * l));
                hi = glm::max(hi, glm::vec2((i + 1) * l, (j + 1) * l));
            }
    if(lo.x > hi.x)
    {
        lo = glm::vec2(leftt, bottomm);
        hi = glm::vec2(rightt, topp);
    }
    glm::vec2 c = (lo + hi) * 0.5f;
    board_center = glm::vec3(c, 0.0f);
    board_radius = max(2.0f, glm::length(hi - lo) * 0.5f);
}

void Camera::initialize_camera()
{
    eye = glm::vec3(0, -6, 6);
    look = glm::vec3(0.0f);
    target = pan = glm::vec3(0.0f);
    heli_eye = eye;
    heli_yaw = 90.0f;
    heli_pitch = -40.0f;
    // Góc mặc định: nhìn chéo từ phía dưới-phải màn, đủ thấp để thấy rõ độ cao của khối.
    follow_orbit = {-65.0f, 56.0f, 7.5f};
    tower_orbit = {-62.0f, 55.0f, 12.0f};
    free_orbit = {-120.0f, 45.0f, 12.0f};
    mouse_orbit = {-65.0f, 50.0f, 8.0f};
    mouse_yaw = -65.0f;
    mouse_pitch = 50.0f;
    ortho_zoom = 1.0f;
    cam_move_dir = 0;
    ori_move_dir = 0;
    proj_type = 1;
    view = follow;
    blend = 1.0f;
    cinematic = false;
    cinematic_yaw = -70.0f;
    drag_button = -1;
    drag_x = drag_y = 0.0;
    snap_next = true;
}

glm::mat4 Camera::get_view()
{
    if(proj_type==1 || cinematic)
        return glm::lookAt(eye, look, UP);
    return glm::lookAt(glm::vec3(0,0,5), glm::vec3(0,0,5) + glm::vec3(0,0,-1), glm::vec3(0,1,0));
}

glm::vec3 Camera::get_eye()
{
    return eye;
}

int Camera::get_view_type()
{
    return view;
}

Orbit *Camera::active_orbit()
{
    if(view == follow) return &follow_orbit;
    if(view == tower) return &tower_orbit;
    if(view == drag_view) return &free_orbit;
    if(view == mouse_view) return &mouse_orbit;
    return NULL;
}

// Điểm khối đang đứng: chỉ lấy x, y của tâm khối (cả khi đang lăn), độ cao cố định
// -> camera không nhấp nhô theo cung lăn của khối.
glm::vec3 Camera::block_focus()
{
    glm::vec3 p = superblock.get_real_pos();
    return glm::vec3(p.x, p.y, 0.35f);
}

void Camera::level_loaded()
{
    compute_board_bounds();
    float fit = clampf(board_radius * 1.15f + 2.5f, 6.0f, 60.0f);
    tower_orbit.dist = fit;
    free_orbit.dist = fit;
    pan = board_center;
    target = block_focus();
    snap_next = true;
}

void Camera::set_cinematic(bool on)
{
    if(on == cinematic) return;
    cinematic = on;
    blend = 0.0f;
    if(on)
    {
        // Bắt đầu quay từ đúng hướng đang nhìn để không bị giật.
        glm::vec3 d = eye - look;
        cinematic_yaw = glm::degrees(atan2(d.y, d.x));
    }
}

void Camera::change_view(int v)
{
    if(v == helicopter && view != helicopter)
    {
        // Bay tiếp từ đúng chỗ đang đứng.
        glm::vec3 d = glm::normalize(look - eye);
        heli_eye = eye;
        heli_yaw = glm::degrees(atan2(d.y, d.x));
        heli_pitch = glm::degrees(asin(clampf(d.z, -1.0f, 1.0f)));
    }
    if(v == mouse_view && view != mouse_view)
    {
        mouse_yaw = mouse_orbit.yaw;
        mouse_pitch = mouse_orbit.pitch;
    }
    view = v;
    proj_type = (v == vertical) ? 0 : 1;
    blend = 0.0f;
    drag_button = -1;
}

void Camera::cam_move(int to)
{
    if(view==helicopter || view==drag_view)
        cam_move_dir = to;
}

void Camera::ori_move(int to)
{
    if(view==helicopter)
        ori_move_dir = to;
}

void Camera::mouse_button(int button, int action)
{
    if(action == GLFW_RELEASE)
    {
        drag_button = -1;
        return;
    }
    if(action != GLFW_PRESS || view == vertical || view == mouse_view || cinematic)
        return;
    drag_button = button == GLFW_MOUSE_BUTTON_LEFT ? 0 : 1;
    glfwGetCursorPos(window, &drag_x, &drag_y);
}

void Camera::scroll(double dy)
{
    if(cinematic || dy == 0.0) return;
    if(view == vertical)
    {
        ortho_zoom = clampf(ortho_zoom * pow(1.12f, float(dy)), 0.4f, 6.0f);
        return;
    }
    if(view == helicopter)
    {
        float y = glm::radians(heli_yaw), p = glm::radians(heli_pitch);
        heli_eye += float(dy) * 0.8f * glm::vec3(cos(p)*cos(y), cos(p)*sin(y), sin(p));
        return;
    }
    Orbit *o = active_orbit();
    if(o) o->dist = clampf(o->dist * pow(0.88f, float(dy)), MIN_DIST, MAX_DIST);
}

void Camera::apply_drag(double dx, double dy)
{
    if(view == helicopter)
    {
        heli_yaw -= float(dx) * 0.2f;
        heli_pitch = clampf(heli_pitch - float(dy) * 0.2f, -85.0f, 85.0f);
        return;
    }
    Orbit *o = active_orbit();
    if(!o) return;
    if(view == drag_view && drag_button == 1)
    {
        // N + chuột phải: dời điểm nhìn trên mặt sàn (kéo sàn theo tay).
        float y = glm::radians(o->yaw);
        glm::vec3 fwd(-cos(y), -sin(y), 0.0f), right(-sin(y), cos(y), 0.0f);
        float k = o->dist * 0.0022f;
        pan -= right * float(dx) * k;
        pan += fwd * float(dy) * k;
        return;
    }
    o->yaw -= float(dx) * 0.30f;
    o->pitch = clampf(o->pitch + float(dy) * 0.25f, MIN_PITCH, MAX_PITCH);
}

void Camera::update(float dt)
{
    // Kéo chuột: đọc vị trí con trỏ mỗi frame (không phụ thuộc callback di chuột).
    if(drag_button >= 0)
    {
        int b = drag_button == 0 ? GLFW_MOUSE_BUTTON_LEFT : GLFW_MOUSE_BUTTON_RIGHT;
        if(glfwGetMouseButton(window, b) != GLFW_PRESS && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) != GLFW_PRESS)
            drag_button = -1;
        else
        {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            apply_drag(x - drag_x, y - drag_y);
            drag_x = x;
            drag_y = y;
        }
    }

    // Điểm bám khối: làm mượt theo hàm mũ (độc lập FPS). Chỉ điểm nhìn trượt, hướng nhìn giữ nguyên.
    glm::vec3 focus = block_focus();
    if(snap_next)
        target = focus;
    else
        target += (1.0f - exp(-7.0f * dt)) * (focus - target);

    // Phím di chuyển (bản gốc: bước cố định mỗi frame ở 60 FPS -> nhân dt*60).
    float f = dt * 60.0f;
    if(view == drag_view && cam_move_dir)
    {
        float y = glm::radians(free_orbit.yaw);
        glm::vec3 fwd(-cos(y), -sin(y), 0.0f), right(-sin(y), cos(y), 0.0f);
        float s = f * camera_move_unit * free_orbit.dist * 0.12f;
        if(cam_move_dir == 'W') pan += fwd * s;
        if(cam_move_dir == 'S') pan -= fwd * s;
        if(cam_move_dir == 'D') pan += right * s;
        if(cam_move_dir == 'A') pan -= right * s;
        if(cam_move_dir == 'T') free_orbit.dist = clampf(free_orbit.dist - s, MIN_DIST, MAX_DIST);
        if(cam_move_dir == 'G') free_orbit.dist = clampf(free_orbit.dist + s, MIN_DIST, MAX_DIST);
    }
    if(view == helicopter)
    {
        float y = glm::radians(heli_yaw), p = glm::radians(heli_pitch);
        glm::vec3 dir(cos(p)*cos(y), cos(p)*sin(y), sin(p));
        glm::vec3 right = glm::normalize(glm::cross(dir, UP));
        glm::vec3 fwd = glm::normalize(glm::cross(UP, right));
        float s = f * camera_move_unit;
        if(cam_move_dir == 'W') heli_eye += fwd * s;
        if(cam_move_dir == 'S') heli_eye -= fwd * s;
        if(cam_move_dir == 'D') heli_eye += right * s;
        if(cam_move_dir == 'A') heli_eye -= right * s;
        if(cam_move_dir == 'T') heli_eye += dir * s;
        if(cam_move_dir == 'G') heli_eye -= dir * s;
        float r = glm::degrees(f * camera_dir_change_unit);
        if(ori_move_dir == 'J') heli_yaw += r;
        if(ori_move_dir == 'L') heli_yaw -= r;
        if(ori_move_dir == 'I') heli_pitch = clampf(heli_pitch + r, -85.0f, 85.0f);
        if(ori_move_dir == 'K') heli_pitch = clampf(heli_pitch - r, -85.0f, 85.0f);
    }
    if(view == mouse_view && !hud_wants_mouse())
    {
        // M: con trỏ ở mép trái/phải = quay hết một vòng, trên/dưới = nhìn từ trên xuống / nhìn ngang.
        int w, h;
        double x, y;
        glfwGetWindowSize(window, &w, &h);
        glfwGetCursorPos(window, &x, &y);
        if(w > 0 && h > 0)
        {
            float cx = clampf(float(x / w), 0.0f, 1.0f), cy = clampf(float(y / h), 0.0f, 1.0f);
            float want_yaw = mouse_orbit.yaw - (cx - 0.5f) * 360.0f;
            float want_pitch = clampf(85.0f - cy * 72.0f, MIN_PITCH, MAX_PITCH);
            float a = 1.0f - exp(-8.0f * dt);
            mouse_yaw += a * (want_yaw - mouse_yaw);
            mouse_pitch += a * (want_pitch - mouse_pitch);
        }
    }

    // Vị trí mong muốn của góc nhìn hiện tại.
    glm::vec3 want_look, want_eye;
    if(cinematic)
    {
        cinematic_yaw += 5.0f * dt;
        want_look = board_center;
        want_eye = want_look + orbit_offset(cinematic_yaw, 34.0f, max(11.0f, tower_orbit.dist * 1.25f));
    }
    else if(view == helicopter)
    {
        float y = glm::radians(heli_yaw), p = glm::radians(heli_pitch);
        want_eye = heli_eye;
        want_look = heli_eye + glm::vec3(cos(p)*cos(y), cos(p)*sin(y), sin(p));
    }
    else if(view == tower)
    {
        want_look = board_center;
        want_eye = want_look + orbit_offset(tower_orbit.yaw, tower_orbit.pitch, tower_orbit.dist);
    }
    else if(view == drag_view)
    {
        want_look = pan;
        want_eye = want_look + orbit_offset(free_orbit.yaw, free_orbit.pitch, free_orbit.dist);
    }
    else if(view == mouse_view)
    {
        want_look = target;
        want_eye = want_look + orbit_offset(mouse_yaw, mouse_pitch, mouse_orbit.dist);
    }
    else
    {
        // follow (C) và các góc cũ không còn dùng: bám khối.
        want_look = target;
        want_eye = want_look + orbit_offset(follow_orbit.yaw, follow_orbit.pitch, follow_orbit.dist);
    }

    if(snap_next)
    {
        eye = want_eye;
        look = want_look;
        snap_next = false;
        blend = 1.0f;
        return;
    }
    // Chuyển góc nhìn: trượt mượt ~0.6 s. Bình thường: làm mượt rất nhẹ cho thao tác kéo/zoom.
    float rate = blend < 1.0f ? 6.0f : 22.0f;
    blend = min(1.0f, blend + dt / 0.6f);
    float a = 1.0f - exp(-rate * dt);
    eye += a * (want_eye - eye);
    look += a * (want_look - look);
}
