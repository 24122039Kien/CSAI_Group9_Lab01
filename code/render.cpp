#include "headers.h"
#include "render.h"
#include "shaders.h"

Renderer renderer;   // theme = 0 (Classic) do biến toàn cục được khởi tạo 0

static GLuint compile_shader(GLenum type, const char *src)
{
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);
    GLint ok = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if(!ok)
    {
        char log[2048];
        glGetShaderInfoLog(id, sizeof(log), NULL, log);
        cerr << "Shader compile error:\n" << log << endl;
    }
    return id;
}

GLuint compile_program(const char *vert_src, const char *frag_src)
{
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vert_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, frag_src);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if(!ok)
    {
        char log[2048];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        cerr << "Shader link error:\n" << log << endl;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

void Renderer::init()
{
    // 6 mặt x 2 tam giác x 3 đỉnh; mỗi đỉnh: vị trí (3) + pháp tuyến (3).
    static const float faces[6][4][3] = {
        {{0,0,0},{0,1,0},{0,1,1},{0,0,1}},   // -x
        {{1,0,0},{1,1,0},{1,1,1},{1,0,1}},   // +x
        {{0,0,0},{1,0,0},{1,0,1},{0,0,1}},   // -y
        {{0,1,0},{1,1,0},{1,1,1},{0,1,1}},   // +y
        {{0,0,0},{1,0,0},{1,1,0},{0,1,0}},   // -z
        {{0,0,1},{1,0,1},{1,1,1},{0,1,1}},   // +z
    };
    static const float normals[6][3] = {{-1,0,0},{1,0,0},{0,-1,0},{0,1,0},{0,0,-1},{0,0,1}};
    static const int quad[6] = {0, 1, 2, 0, 2, 3};

    vector<float> data;
    for(int f=0; f<6; f++)
        for(int k=0; k<6; k++)
        {
            const float *p = faces[f][quad[k]];
            data.insert(data.end(), p, p+3);
            data.insert(data.end(), normals[f], normals[f]+3);
        }

    glGenVertexArrays(1, &box_vao);
    glGenBuffers(1, &box_vbo);
    glBindVertexArray(box_vao);
    glBindBuffer(GL_ARRAY_BUFFER, box_vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size()*sizeof(float), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    // Buffer dữ liệu theo hộp: mat4 (4 cột, location 2..5) + 4 vec4 (location 6..9), divisor 1.
    glGenBuffers(1, &inst_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, inst_vbo);
    for(int k=0; k<8; k++)
    {
        glVertexAttribPointer(2+k, 4, GL_FLOAT, GL_FALSE, sizeof(BoxInstance), (void*)(k*4*sizeof(float)));
        glEnableVertexAttribArray(2+k);
        glVertexAttribDivisor(2+k, 1);
    }

    glGenVertexArrays(1, &bg_vao);   // tam giác nền không cần buffer (đỉnh sinh từ gl_VertexID)

    box_prog = compile_program(BOX_VERT, BOX_FRAG);
    u_viewproj  = glGetUniformLocation(box_prog, "uViewProj");
    u_time      = glGetUniformLocation(box_prog, "uTime");
    u_eye       = glGetUniformLocation(box_prog, "uEye");
    u_light     = glGetUniformLocation(box_prog, "uLightDir");
    u_fogcolor  = glGetUniformLocation(box_prog, "uFogColor");
    u_fogrange  = glGetUniformLocation(box_prog, "uFogRange");
    u_theme     = glGetUniformLocation(box_prog, "uTheme");

    bg_prog = compile_program(BG_VERT, BG_FRAG);
    ubg_time = glGetUniformLocation(bg_prog, "uTime");
    ubg_res  = glGetUniformLocation(bg_prog, "uRes");
    ubg_theme = glGetUniformLocation(bg_prog, "uTheme");
    draw_calls = 0;
    boxes = 0;
}

void Renderer::draw_background(int fbwidth, int fbheight, float time)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glUseProgram(bg_prog);
    glUniform1f(ubg_time, time);
    glUniform2f(ubg_res, (float)fbwidth, (float)fbheight);
    glUniform1i(ubg_theme, theme);
    glBindVertexArray(bg_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    draw_calls++;
}

void Renderer::begin(const glm::mat4 &view_proj, const glm::vec3 &eye, float time, float fog_near, float fog_far)
{
    glUseProgram(box_prog);
    glBindVertexArray(box_vao);
    glUniformMatrix4fv(u_viewproj, 1, GL_FALSE, &view_proj[0][0]);
    glUniform3fv(u_eye, 1, &eye[0]);
    glm::vec3 light = glm::normalize(glm::vec3(-0.35f, 0.55f, -1.0f));
    glUniform3fv(u_light, 1, &light[0]);
    if(theme == THEME_MINECRAFT)
        glUniform3f(u_fogcolor, 0.72f, 0.84f, 1.0f);   // sương hòa vào bầu trời
    else
        glUniform3f(u_fogcolor, 0.01f, 0.012f, 0.03f);
    glUniform1i(u_theme, theme);
    glUniform2f(u_fogrange, fog_near, fog_far);
    glUniform1f(u_time, time);
}

void Renderer::draw_box(const glm::mat4 &model, const glm::vec3 &size, const BoxStyle &style)
{
    BoxInstance b;
    b.model = model;
    b.color = glm::vec4(style.color, style.emissive);
    b.accent = glm::vec4(style.accent, style.alpha);
    b.scale = glm::vec4(glm::abs(size), float(style.pattern));
    b.extra = glm::vec4(style.unlit ? 1.0f : 0.0f, float(style.material), 0.0f, 0.0f);
    batch.push_back(b);
}

void Renderer::flush()
{
    if(batch.empty())
        return;
    glUseProgram(box_prog);
    glBindVertexArray(box_vao);
    glBindBuffer(GL_ARRAY_BUFFER, inst_vbo);
    // Cấp lại buffer mỗi lần (orphaning) để driver không phải chờ frame trước vẽ xong.
    glBufferData(GL_ARRAY_BUFFER, batch.size()*sizeof(BoxInstance), batch.data(), GL_STREAM_DRAW);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, (GLsizei)batch.size());
    draw_calls++;
    boxes += (int)batch.size();
    batch.clear();
}

void Renderer::draw_aabb(const glm::vec3 &pos, const glm::vec3 &size, const BoxStyle &style)
{
    draw_box(glm::translate(pos) * glm::scale(size), size, style);
}
