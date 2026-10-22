void error_callback(int error, const char* description);
void quit(GLFWwindow *window);
// Mở file theo đường dẫn UTF-8 (Windows: chuyển sang UTF-16 và dùng _wfopen để đường dẫn có dấu vẫn mở được).
FILE *utf8_fopen(const string &path, const char *mode);
bool read_text_file(const string &path, string &out);
