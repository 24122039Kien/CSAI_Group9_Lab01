# Bloxorz 3D (C++/OpenGL 3.3) - build bằng GNU make, chạy trong thư mục code/
#
#   Linux / WSL : sudo apt install g++ make libglfw3-dev libglm-dev libgl-dev
#   macOS       : brew install glfw glm
#   Windows     : MSYS2 MinGW64 -> pacman -S make zip mingw-w64-x86_64-{gcc,glfw,glm}
#
#   make          build bản dev ./a.out (Windows: a.exe, có console để xem log)
#   make release  build bản phát hành (Windows: Bloxorz.exe tĩnh, không console, có icon; Linux: ./bloxorz)
#   make dist     đóng gói ../dist/<nền tảng>/Bloxorz/ (exe + data + solver Python), nén thành ../dist/Bloxorz-<nền tảng>.zip
#   make clean    xóa file build

CXX      ?= g++
CC       ?= gcc
CXXFLAGS ?= -std=c++11 -O2
CFLAGS   ?= -O2
CPPFLAGS += -Iinclude -Ithird_party/imgui

UNAME_S := $(shell uname -s 2>/dev/null)

ifeq ($(UNAME_S),Linux)
    EXEC     := a.out
    APP      := bloxorz
    PLATFORM := linux
    LDLIBS   := -lglfw -lGL -ldl -lpthread
else ifeq ($(UNAME_S),Darwin)
    EXEC     := a.out
    APP      := bloxorz
    PLATFORM := macos
    BREW     := $(shell brew --prefix 2>/dev/null)
    CPPFLAGS += -I$(BREW)/include
    LDFLAGS  += -L$(BREW)/lib
    LDLIBS   := -lglfw -framework OpenGL
else
    # Windows: MSYS2/MinGW (uname = MINGW64_NT-..., MSYS_NT-...)
    EXEC     := a.exe
    APP      := Bloxorz.exe
    PLATFORM := win64
    LDLIBS   := -lglfw3 -lopengl32 -lgdi32 -limm32 -lshell32
    RES      := build/win64/bloxorz.res.o
    # exe tự chứa (không cần DLL của MinGW/GLFW) và không mở cửa sổ console
    RELEASE_LDFLAGS := -static -static-libgcc -static-libstdc++ -mwindows
endif

# Object để riêng theo nền tảng: build trong WSL và MSYS2 trên cùng thư mục không lẫn .o của nhau
OBJDIR    := build/$(PLATFORM)
IMGUI_DIR := third_party/imgui
IMGUI_SRC := imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui_impl_glfw.cpp imgui_impl_opengl3.cpp
IMGUI_OBJ := $(addprefix $(OBJDIR)/imgui/,$(IMGUI_SRC:.cpp=.o))

SRC := $(wildcard *.cpp)
OBJ := $(addprefix $(OBJDIR)/,$(SRC:.cpp=.o)) $(OBJDIR)/glad.o $(IMGUI_OBJ)
HDR := $(wildcard *.h)

DIST_ROOT := ../dist
DIST      := $(DIST_ROOT)/$(PLATFORM)/Bloxorz

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

release: $(APP)

$(APP): $(OBJ) $(RES)
	$(CXX) $(LDFLAGS) $(RELEASE_LDFLAGS) $^ -o $@ $(LDLIBS)

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

build/win64/bloxorz.res.o: bloxorz.rc assets/bloxorz.ico
	@mkdir -p $(@D)
	windres $< -O coff -o $@

dist: $(APP)
	rm -rf $(DIST) $(DIST_ROOT)/Bloxorz-$(PLATFORM).*
	mkdir -p $(DIST)/data $(DIST)/solver $(DIST)/licenses
	cp $(APP) $(DIST)/
	cp data/*.data $(DIST)/data/
	cp ../main.py $(DIST)/solver/
	cp -r ../ai $(DIST)/solver/
	find $(DIST)/solver -name __pycache__ -prune -exec rm -rf {} +
	cp assets/README_dist.txt $(DIST)/README.txt
	cp ../THIRD_PARTY_NOTICES.md $(DIST)/licenses/
	cp $(IMGUI_DIR)/LICENSE.txt $(DIST)/licenses/imgui_LICENSE.txt
	cd $(DIST_ROOT)/$(PLATFORM) && if command -v zip >/dev/null 2>&1; then zip -qr ../Bloxorz-$(PLATFORM).zip Bloxorz; \
	    else tar czf ../Bloxorz-$(PLATFORM).tar.gz Bloxorz; fi
	@echo "Packaged $(DIST) -> $(DIST_ROOT)/Bloxorz-$(PLATFORM).*"

clean:
	rm -rf build *.o a.out a.exe bloxorz Bloxorz.exe solution.txt solver_log.txt

.PHONY: all release dist clean
