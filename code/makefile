# Bloxorz 3D (C++/OpenGL 3.3) - build bằng GNU make, chạy trong thư mục code/
#
#   Linux / WSL : sudo apt install g++ make libglfw3-dev libglm-dev libgl-dev
#   macOS       : brew install glfw glm
#   Windows     : MSYS2 MinGW64 -> pacman -S make mingw-w64-x86_64-{gcc,glfw,glm}
#
#   make          build bản dev ./a.out (Windows: a.exe, có console để xem log)
#   make clean    xóa file build

CXX      ?= g++
CC       ?= gcc
CXXFLAGS ?= -std=c++11 -O2
CFLAGS   ?= -O2
CPPFLAGS += -Iinclude -Ithird_party/imgui

UNAME_S := $(shell uname -s 2>/dev/null)

ifeq ($(UNAME_S),Linux)
    EXEC     := a.out
    PLATFORM := linux
    LDLIBS   := -lglfw -lGL -ldl -lpthread
else ifeq ($(UNAME_S),Darwin)
    EXEC     := a.out
    PLATFORM := macos
    BREW     := $(shell brew --prefix 2>/dev/null)
    CPPFLAGS += -I$(BREW)/include
    LDFLAGS  += -L$(BREW)/lib
    LDLIBS   := -lglfw -framework OpenGL
else
    # Windows: MSYS2/MinGW (uname = MINGW64_NT-..., MSYS_NT-...)
    EXEC     := a.exe
    PLATFORM := win64
    LDLIBS   := -lglfw3 -lopengl32 -lgdi32 -limm32 -lshell32
endif

# Object để riêng theo nền tảng: build trong WSL và MSYS2 trên cùng thư mục không lẫn .o của nhau
OBJDIR    := build/$(PLATFORM)
IMGUI_DIR := third_party/imgui
IMGUI_SRC := imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui_impl_glfw.cpp imgui_impl_opengl3.cpp
IMGUI_OBJ := $(addprefix $(OBJDIR)/imgui/,$(IMGUI_SRC:.cpp=.o))

SRC := $(wildcard *.cpp)
OBJ := $(addprefix $(OBJDIR)/,$(SRC:.cpp=.o)) $(OBJDIR)/glad.o $(IMGUI_OBJ)
HDR := $(wildcard *.h)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJDIR)/%.o: %.cpp $(HDR)
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# ImGui không phụ thuộc header của game -> không phải biên dịch lại khi sửa game
$(OBJDIR)/imgui/%.o: $(IMGUI_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/glad.o: glad.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build *.o a.out a.exe solution.txt solver_log.txt

.PHONY: all clean
