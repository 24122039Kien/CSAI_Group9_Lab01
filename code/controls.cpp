#include "headers.h"
#include "noedit.h"
#include "camera.h"
#include "superblock.h"
#include "game.h"
#include "autoplay.h"
#include "app.h"
#include "hud.h"

glm::mat4 ortho_fit(float aspect);

int proj_type;
float nearr = 0.1f;
float farr = 500.0f;
float leftt = -4.0f;
float rightt = 4.0f;
float topp = 4.0f;
float bottomm = -4.0f;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Esc luôn xử lý (mở/đóng menu); các phím khác nhường cho menu ImGui khi nó đang nhận phím.
    if (key != GLFW_KEY_ESCAPE && hud_wants_keyboard())
        return;
    if (action == GLFW_RELEASE)
    {
        switch(key)
        {
            case GLFW_KEY_SPACE:
                proj_type ^= 1;
                break;
            case GLFW_KEY_X:
            case GLFW_KEY_C:
            case GLFW_KEY_V:
            case GLFW_KEY_B:
            case GLFW_KEY_N:
            case GLFW_KEY_M:
                camera.change_view(key);
            case GLFW_KEY_A:
            case GLFW_KEY_S:
            case GLFW_KEY_W:
            case GLFW_KEY_D:
            case GLFW_KEY_T:
            case GLFW_KEY_G:
                camera.cam_move(0);
                break;
            case GLFW_KEY_I:
            case GLFW_KEY_K:
            case GLFW_KEY_J:
            case GLFW_KEY_L:
                camera.ori_move(0);
                break;
            case GLFW_KEY_UP:
            case GLFW_KEY_DOWN:
            case GLFW_KEY_LEFT:
            case GLFW_KEY_RIGHT:
                // Khóa điều khiển tay khi đang phát lại lời giải, ván đã kết thúc, đang mở màn hoặc mở menu.
                if(!app_input_locked())
                    superblock.move(key);
                break;
            case GLFW_KEY_TAB:
                show_minimap = !show_minimap;
                break;
            case GLFW_KEY_F1:
                show_help = !show_help;
                break;
            case GLFW_KEY_F12:
                app_request_screenshot();
                break;
            case GLFW_KEY_R:
                if(!cli_mode)
                    app_restart();
                break;
            case GLFW_KEY_ENTER:
            case GLFW_KEY_KP_ENTER:
                if(!cli_mode && game_result == 1)
                    app_next_level();
                else if(!cli_mode && game_result == 0)
                    app_restart();
                break;
        	default:
        	    break;
        }
    }
    else if (action == GLFW_PRESS)
    {
        switch(key)
        {
        	case GLFW_KEY_ESCAPE:
        	    if(cli_mode)
        	        quit(window);
        	    else
        	        app_toggle_menu();
        	    break;
            case GLFW_KEY_A:
            case GLFW_KEY_S:
            case GLFW_KEY_W:
            case GLFW_KEY_D:
            case GLFW_KEY_T:
            case GLFW_KEY_G:
                camera.cam_move(key);
                break;
            case GLFW_KEY_I:
            case GLFW_KEY_K:
            case GLFW_KEY_J:
            case GLFW_KEY_L:
                camera.ori_move(key);
        	default:
        	    break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key)
    {
        case 'Q':
        case 'q':
    	   quit(window);
    	   break;

        default:
	       break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    if (hud_wants_mouse())
        return;

	if(action == GLFW_RELEASE)
    {
        switch (button)
        {
            case GLFW_MOUSE_BUTTON_LEFT:
            camera.left_button_release();
               break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                break;

            default:
                break;
        }
    }
	else if(action == GLFW_PRESS)
    {
        switch (button)
        {
            case GLFW_MOUSE_BUTTON_LEFT:
                camera.left_button_press();
               break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                break;

            default:
                break;
        }
    }

}

/*
void calculate_perspective()
{
    //int fbwidth=width, fbheight=height;
    int fbwidth, fbheight;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = M_PI/2;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    Matrices.projectionP = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, nearr, farr);
}
void calculate_ortho()
{
    // Ortho projection for 2D views
    Matrices.projectionO = glm::ortho(leftt, rightt, bottomm, topp, nearr, farr);
}
*/
/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    if(fbwidth <= 0 || fbheight <= 0)
        return;   // cửa sổ bị thu nhỏ
    GLfloat fov = glm::radians(75.0f);   // bản gốc 90 độ: méo hình ở mép
    GLfloat aspect = (GLfloat) fbwidth / (GLfloat) fbheight;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    Matrices.projectionP = glm::perspective(fov, aspect, nearr, farr);
    // Ortho projection for 2D views: giữ đúng tỉ lệ, đặt màn chơi ở giữa
    Matrices.projectionO = ortho_fit(aspect);

}

glm::mat4 ortho_fit(float aspect)
{
    float cx = (leftt + rightt) / 2, cy = (bottomm + topp) / 2;
    float half = max(rightt - leftt, topp - bottomm) / 2;
    if(aspect >= 1.0f)
        return glm::ortho(cx - half*aspect, cx + half*aspect, cy - half, cy + half, nearr, farr);
    return glm::ortho(cx - half, cx + half, cy - half/aspect, cy + half/aspect, nearr, farr);
}

void scroll(GLFWwindow* window, double x, double y)
{
    if(hud_wants_mouse())return;
    int i = camera.get_view_type();
    if(i!=drag_view && i!= mouse_view)return;

    x = int(x);
    y = int(y);
    if(x==0)camera.zoom_change(-y);
}


void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    int i = camera.get_view_type();
    if(i!=helicopter)return;

	int _w, _h;
    double x, y, w, h;
    glfwGetCursorPos (window, &x, &y);
    glfwGetWindowSize(window, &_w, &_h);

    w = _w;
    h = _h;
    x -= w/2;
    y -= h/2;
    y = -y;

    x = 2.0f*x/w;
    y = 2.0f*y/h;
    camera.dir_change(x, y);
}
