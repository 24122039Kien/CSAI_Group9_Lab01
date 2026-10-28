#ifndef CAMERA
#define CAMERA

// Camera quỹ đạo (orbit): mắt nằm trên mặt cầu quanh một điểm nhìn, xác định bởi
// yaw (góc ngang), pitch (góc ngẩng) và dist (khoảng cách). Trục "lên" luôn là +z nên không bao giờ nghiêng.
//   C follow   : điểm nhìn bám khối (làm mượt), hướng nhìn giữ nguyên -> chỉ tịnh tiến, không lắc
//   B tower    : nhìn toàn màn từ trên cao
//   N orbit    : xoay tự do quanh màn, kéo chuột phải / WASD để dời điểm nhìn
//   M mouse    : góc nhìn đi theo vị trí con trỏ (không cần bấm), xoay quanh khối
//   X heli     : bay tự do (WASD TG di chuyển, IJKL xoay)
//   V vertical : nhìn thẳng từ trên, phép chiếu song song
// Mọi góc nhìn 3D: kéo chuột trái (hoặc phải) để xoay, lăn chuột để zoom.
struct Orbit
{
    float yaw, pitch, dist;   // độ, độ, đơn vị thế giới
};

class Camera
{
    private:
        glm::vec3 eye, look;          // vị trí mắt / điểm nhìn đang dùng (đã làm mượt)
        glm::vec3 target;             // điểm nhìn của C/M (bám khối) đã làm mượt
        glm::vec3 pan;                // điểm nhìn của N (dời được)
        glm::vec3 heli_eye;           // X: vị trí bay
        float heli_yaw, heli_pitch;
        Orbit follow_orbit, tower_orbit, free_orbit, mouse_orbit;
        float mouse_yaw, mouse_pitch; // M: góc mục tiêu theo con trỏ (làm mượt)
        float ortho_zoom;             // V: > 1 phóng to
        int cam_move_dir;
        int ori_move_dir;
        int view;
        float blend;                  // 0..1: đang chuyển cảnh mượt giữa hai góc nhìn
        bool cinematic;               // màn hình chính: camera tự quay chậm quanh màn
        float cinematic_yaw;
        int drag_button;              // -1 không kéo, 0 trái, 1 phải
        double drag_x, drag_y;
        bool snap_next;               // frame sau đặt thẳng vào vị trí (không trượt)
        Orbit *active_orbit();
        glm::vec3 block_focus();
        void apply_drag(double dx, double dy);
    public:
        void initialize_camera();
        glm::mat4 get_view();
        void cam_move(int to);
        void ori_move(int to);
        void update(float dt);
        glm::vec3 get_eye();
        void change_view(int v);
        int get_view_type();
        void mouse_button(int button, int action);
        void scroll(double dy);
        void level_loaded();          // màn mới: tính lại camera tower, đặt camera bám ngay vào khối
        void set_cinematic(bool on);
        float get_ortho_zoom() { return ortho_zoom; }
        bool is_dragging() { return drag_button >= 0; }
};

#endif

extern Camera camera;
extern float camera_move_unit;
extern float camera_dir_change_unit;
extern int helicopter, vertical, tower, follow, block_view, mouse_view, mouse_view_cs, drag_view;
