GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
void error_callback(int error, const char* description);
void quit(GLFWwindow *window);
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL);
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL);
void draw3DObject (struct VAO* vao);
// Mở file theo đường dẫn UTF-8 (Windows: chuyển sang UTF-16 và dùng _wfopen để đường dẫn có dấu vẫn mở được).
FILE *utf8_fopen(const string &path, const char *mode);
bool read_text_file(const string &path, string &out);
