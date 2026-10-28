#include "headers.h"
#include "render.h"
#include "shaders.h"

Renderer renderer;   // theme = 0 (Classic) do biến toàn cục được khởi tạo 0

// Khung bao các ô của màn hiện tại (camera.cpp tính khi nạp màn): dùng để đặt vùng chiếu của shadow map.
extern glm::vec3 board_center;
extern float board_radius;

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
    u_lightvp   = glGetUniformLocation(box_prog, "uLightVP");
    u_shadowon  = glGetUniformLocation(box_prog, "uShadowOn");
    glUseProgram(box_prog);
    glUniform1i(glGetUniformLocation(box_prog, "uShadowMap"), 3);   // shadow map luôn ở texture unit 3

    // Shadow map: texture độ sâu có chế độ so sánh (sampler2DShadow), lọc tuyến tính -> PCF phần cứng.
    shadow_prog = compile_program(SHADOW_VERT, SHADOW_FRAG);
    us_lightvp = glGetUniformLocation(shadow_prog, "uLightVP");
    shadow_size = 2048;
    glGenTextures(1, &shadow_tex);
    glBindTexture(GL_TEXTURE_2D, shadow_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadow_size, shadow_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glGenFramebuffers(1, &shadow_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_tex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        cerr << "Shadow map framebuffer incomplete - shadows disabled" << endl;
        shadow_size = 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    shadow_now = false;
    light_vp = glm::mat4(1.0f);

    // Hậu kỳ (framebuffer tạo khi biết kích thước cửa sổ, trong begin_frame).
    post_bright = compile_program(BG_VERT, POST_BRIGHT_FRAG);
    post_blur = compile_program(BG_VERT, POST_BLUR_FRAG);
    post_comp = compile_program(BG_VERT, POST_COMPOSITE_FRAG);
    up_blur_dir = glGetUniformLocation(post_blur, "uDir");
    up_comp_strength = glGetUniformLocation(post_comp, "uBloomStrength");
    up_comp_theme = glGetUniformLocation(post_comp, "uTheme");
    glUseProgram(post_comp);
    glUniform1i(glGetUniformLocation(post_comp, "uScene"), 0);
    glUniform1i(glGetUniformLocation(post_comp, "uBloom1"), 1);
    glUniform1i(glGetUniformLocation(post_comp, "uBloom2"), 2);
    scene_fbo = scene_color_rb = scene_depth_rb = resolve_fbo = resolve_tex = 0;
    for(int k=0; k<4; k++) bloom_fbo[k] = bloom_tex[k] = 0;
    post_w = post_h = 0;
    post_failed = false;
    post_active = false;

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

glm::vec3 Renderer::light_dir()
{
    if(theme == THEME_ORIGINAL)
        return glm::normalize(glm::vec3(-0.55f, 0.40f, -0.95f));   // nắng chiều xiên hơn: bóng dài
    return glm::normalize(glm::vec3(-0.35f, 0.55f, -1.0f));
}

void Renderer::begin(const glm::mat4 &view_proj, const glm::vec3 &eye, float time, float fog_near, float fog_far,
                     bool main_view)
{
    glUseProgram(box_prog);
    glBindVertexArray(box_vao);
    glUniformMatrix4fv(u_viewproj, 1, GL_FALSE, &view_proj[0][0]);
    glUniform3fv(u_eye, 1, &eye[0]);
    glm::vec3 light = light_dir();
    glUniform3fv(u_light, 1, &light[0]);
    if(theme == THEME_MINECRAFT)
        glUniform3f(u_fogcolor, 0.72f, 0.84f, 1.0f);   // sương hòa vào bầu trời
    else if(theme == THEME_ORIGINAL)
        glUniform3f(u_fogcolor, 0.42f, 0.07f, 0.03f);  // hòa vào trời đỏ
    else
        glUniform3f(u_fogcolor, 0.01f, 0.012f, 0.03f);

    // Nguồn sáng song song: hộp chiếu trực giao bao trọn màn chơi, đặt lùi ngược hướng tia sáng.
    shadow_now = main_view && shadows && shadow_size > 0;
    if(shadow_now)
    {
        float r = board_radius + 2.5f;
        glm::vec3 c = board_center;
        glm::mat4 lv = glm::lookAt(c - light * (r + 25.0f), c, glm::vec3(0.0f, 0.0f, 1.0f));
        light_vp = glm::ortho(-r, r, -r, r, 1.0f, 2.0f * r + 55.0f) * lv;
        glUniformMatrix4fv(u_lightvp, 1, GL_FALSE, &light_vp[0][0]);
    }
    glUniform1i(u_shadowon, shadow_now ? 1 : 0);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, shadow_tex);
    glActiveTexture(GL_TEXTURE0);
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

void Renderer::upload_batch()
{
    glBindVertexArray(box_vao);
    glBindBuffer(GL_ARRAY_BUFFER, inst_vbo);
    // Cấp lại buffer mỗi lần (orphaning) để driver không phải chờ frame trước vẽ xong.
    glBufferData(GL_ARRAY_BUFFER, batch.size()*sizeof(BoxInstance), batch.data(), GL_STREAM_DRAW);
}

void Renderer::flush()
{
    if(batch.empty())
        return;
    glUseProgram(box_prog);
    upload_batch();
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, (GLsizei)batch.size());
    draw_calls++;
    boxes += (int)batch.size();
    batch.clear();
}

void Renderer::flush_opaque()
{
    if(!shadow_now || batch.empty())
    {
        flush();
        return;
    }
    upload_batch();
    GLint prev_fbo = 0, vp[4];
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prev_fbo);
    glGetIntegerv(GL_VIEWPORT, vp);

    // Lượt 1: độ sâu nhìn từ nguồn sáng. Polygon offset tránh "mụn bóng" trên chính bề mặt.
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    glViewport(0, 0, shadow_size, shadow_size);
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(shadow_prog);
    glUniformMatrix4fv(us_lightvp, 1, GL_FALSE, &light_vp[0][0]);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.5f, 3.0f);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, (GLsizei)batch.size());
    glDisable(GL_POLYGON_OFFSET_FILL);

    // Lượt 2: cảnh bình thường, đọc shadow map ở unit 3.
    glBindFramebuffer(GL_FRAMEBUFFER, prev_fbo);
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    glUseProgram(box_prog);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, shadow_tex);
    glActiveTexture(GL_TEXTURE0);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, (GLsizei)batch.size());
    draw_calls += 2;
    boxes += (int)batch.size();
    batch.clear();
}

/*************
 *  Hậu kỳ   *
 *************/

static GLuint make_color_tex(int w, int h)
{
    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return t;
}

static GLuint make_fbo_for(GLuint tex)
{
    GLuint f;
    glGenFramebuffers(1, &f);
    glBindFramebuffer(GL_FRAMEBUFFER, f);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    return f;
}

void Renderer::release_post()
{
    if(scene_fbo) glDeleteFramebuffers(1, &scene_fbo);
    if(resolve_fbo) glDeleteFramebuffers(1, &resolve_fbo);
    if(scene_color_rb) glDeleteRenderbuffers(1, &scene_color_rb);
    if(scene_depth_rb) glDeleteRenderbuffers(1, &scene_depth_rb);
    if(resolve_tex) glDeleteTextures(1, &resolve_tex);
    for(int k=0; k<4; k++)
    {
        if(bloom_fbo[k]) glDeleteFramebuffers(1, &bloom_fbo[k]);
        if(bloom_tex[k]) glDeleteTextures(1, &bloom_tex[k]);
        bloom_fbo[k] = bloom_tex[k] = 0;
    }
    scene_fbo = resolve_fbo = scene_color_rb = scene_depth_rb = resolve_tex = 0;
    post_w = post_h = 0;
}

bool Renderer::ensure_post(int w, int h)
{
    if(w == post_w && h == post_h && scene_fbo)
        return true;
    release_post();
    GLint max_samples = 4;
    glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
    int samples = max_samples < 4 ? (int)max_samples : 4;

    // Framebuffer cảnh: màu HDR (16-bit float) + độ sâu, cùng có MSAA để giữ khử răng cưa như trước.
    glGenRenderbuffers(1, &scene_color_rb);
    glBindRenderbuffer(GL_RENDERBUFFER, scene_color_rb);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA16F, w, h);
    glGenRenderbuffers(1, &scene_depth_rb);
    glBindRenderbuffer(GL_RENDERBUFFER, scene_depth_rb);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, w, h);
    glGenFramebuffers(1, &scene_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, scene_color_rb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, scene_depth_rb);
    bool ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    resolve_tex = make_color_tex(w, h);
    resolve_fbo = make_fbo_for(resolve_tex);
    ok = ok && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    // Bloom 2 tầng: 1/2 và 1/4 độ phân giải, mỗi tầng một cặp texture để làm nhòe qua lại.
    for(int k=0; k<4; k++)
    {
        int d = k < 2 ? 2 : 4;
        bloom_tex[k] = make_color_tex(max(1, w / d), max(1, h / d));
        bloom_fbo[k] = make_fbo_for(bloom_tex[k]);
        ok = ok && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if(!ok)
    {
        cerr << "Post-processing framebuffer incomplete - bloom disabled" << endl;
        release_post();
        post_failed = true;
        return false;
    }
    post_w = w;
    post_h = h;
    return true;
}

void Renderer::begin_frame(int fbwidth, int fbheight)
{
    post_active = bloom && !post_failed && ensure_post(fbwidth, fbheight);
    glBindFramebuffer(GL_FRAMEBUFFER, post_active ? scene_fbo : 0);
}

void Renderer::fullscreen_pass(GLuint prog, GLuint target_fbo, int w, int h)
{
    glBindFramebuffer(GL_FRAMEBUFFER, target_fbo);
    glViewport(0, 0, w, h);
    glUseProgram(prog);
    glBindVertexArray(bg_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    draw_calls++;
}

void Renderer::end_frame(int fbwidth, int fbheight)
{
    if(!post_active)
        return;
    int w = post_w, h = post_h;
    int hw = max(1, w / 2), hh = max(1, h / 2), qw = max(1, w / 4), qh = max(1, h / 4);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);

    // Gộp mẫu MSAA thành một texture thường.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, scene_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_fbo);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, resolve_tex);
    fullscreen_pass(post_bright, bloom_fbo[0], hw, hh);              // lọc vùng sáng, thu nhỏ 1/2
    glUseProgram(post_blur);
    for(int pass = 0; pass < 2; pass++)
    {
        glBindTexture(GL_TEXTURE_2D, bloom_tex[0]);
        glUseProgram(post_blur);
        glUniform2f(up_blur_dir, 1.0f / hw, 0.0f);
        fullscreen_pass(post_blur, bloom_fbo[1], hw, hh);
        glBindTexture(GL_TEXTURE_2D, bloom_tex[1]);
        glUniform2f(up_blur_dir, 0.0f, 1.0f / hh);
        fullscreen_pass(post_blur, bloom_fbo[0], hw, hh);
    }
    for(int pass = 0; pass < 2; pass++)                               // tầng 1/4: quầng rộng hơn
    {
        glBindTexture(GL_TEXTURE_2D, pass == 0 ? bloom_tex[0] : bloom_tex[3]);
        glUniform2f(up_blur_dir, pass == 0 ? 1.0f / hw : 1.0f / qw, 0.0f);
        fullscreen_pass(post_blur, bloom_fbo[2], qw, qh);
        glBindTexture(GL_TEXTURE_2D, bloom_tex[2]);
        glUniform2f(up_blur_dir, 0.0f, 1.0f / qh);
        fullscreen_pass(post_blur, bloom_fbo[3], qw, qh);
    }

    // Ghép: cảnh + bloom -> framebuffer mặc định (HUD vẽ đè sau đó).
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, resolve_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloom_tex[0]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, bloom_tex[3]);
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(post_comp);
    glUniform1f(up_comp_strength, theme == THEME_MINECRAFT ? 0.45f : 0.9f);
    glUniform1i(up_comp_theme, theme);
    fullscreen_pass(post_comp, 0, fbwidth, fbheight);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, fbwidth, fbheight);
}

void Renderer::draw_aabb(const glm::vec3 &pos, const glm::vec3 &size, const BoxStyle &style)
{
    draw_box(glm::translate(pos) * glm::scale(size), size, style);
}
