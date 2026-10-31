#include "colors.h"

#ifndef SUPERBLOCK
#define SUPERBLOCK
class Game;

class SuperBlock
{
    private:
        glm::vec3 pos, real_pos;
        glm::vec3 dim;
        Color color;
        int along;
        int rot_dir;
        int last_dir;                // hướng lăn gần nhất (cho hiệu ứng rơi)
        float rem_rotation, done_rotation;
        int ix, iy;
        int end_anim;                // hiệu ứng kết thúc: 0 không, 1 chìm vào lỗ, 2 rơi
        double end_start;
        glm::vec2 fall_dir;
        glm::mat4 model;             // ma trận vẽ của frame gần nhất (dùng cho bóng)
        glm::mat4 face_sym;          // đối xứng của hộp 2x1x1: hướng thật so với tư thế chuẩn (mặt nào ở đâu)
        glm::mat4 cube_rot[2];       // hướng thật của từng khối nhỏ sau khi tách
        void land_dust();
    public:
        bool is_split;
        int active_cube;
        int falling_cube;
        int cube_ix[2], cube_iy[2];
        glm::vec3 cube_pos[2];
        glm::mat4 cube_model[2];
        int buffered_move;

        friend class Game;
        SuperBlock();
        SuperBlock(glm::vec3 pos, glm::vec3 dim, Color c, int a);
        void initialize(glm::vec3 pos, glm::vec3 dim, Color c, int a);
        void draw(glm::mat4 M);
        void draw_shadow();
        void move(int m);
        void update(float dt);       // dt: số giây kể từ frame trước
        bool is_moving();   // true khi khối đang trong animation lăn
        glm::vec3 get_real_pos();
        int get_along() { return along; }
        int get_ix() { return ix; }
        int get_iy() { return iy; }
        void start_sink(double now);
        void start_fall(double now);
        void split(int x1, int y1, int x2, int y2);
        void switch_active_cube();
        bool check_recombine();
        bool get_is_split() { return is_split; }
        int get_active_cube() { return active_cube; }
        ~SuperBlock();
};

extern int X,Y,Z;
extern int U, D, R, L, NO;
extern float roll_speed;
extern SuperBlock superblock;

#endif
