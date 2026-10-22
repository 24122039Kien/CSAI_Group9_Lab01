#ifndef SCREENSHOT
#define SCREENSHOT

// Lưu framebuffer hiện tại (trước khi swap) thành file PNG.
bool save_framebuffer_png(const string &path, int width, int height);

#endif
