#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "headers.h"
#include "noedit.h"

// Shader giờ được nhúng và biên dịch trong render.cpp (compile_program); VAO riêng cho từng
// vật thể đã được thay bằng một mesh hộp dùng chung (Renderer).

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

FILE *utf8_fopen(const string &path, const char *mode)
{
#ifdef _WIN32
    int n = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, NULL, 0);
    std::wstring wpath(n > 0 ? n : 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &wpath[0], n);
    std::wstring wmode(mode, mode + strlen(mode));
    return _wfopen(wpath.c_str(), wmode.c_str());
#else
    return fopen(path.c_str(), mode);
#endif
}

bool read_text_file(const string &path, string &out)
{
    FILE *f = utf8_fopen(path, "rb");
    if(!f)
        return false;
    out.clear();
    char buf[4096];
    size_t k;
    while((k = fread(buf, 1, sizeof(buf), f)) > 0)
        out.append(buf, k);
    fclose(f);
    return true;
}
