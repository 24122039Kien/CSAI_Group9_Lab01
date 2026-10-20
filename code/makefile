# Bloxorz 3D (C++/OpenGL 3.3) - build bằng GNU make, chạy trong thư mục code/
#
#   Linux / WSL : sudo apt install g++ make libglfw3-dev libglm-dev libgl-dev
#   macOS       : brew install glfw glm
#   Windows     : MSYS2 MinGW64 -> pacman -S make mingw-w64-x86_64-{gcc,glfw,glm}
#
#   make          build ra ./a.out (Windows: a.exe)
#   make clean    xóa file build

CXX      ?= g++
CC       ?= gcc
CXXFLAGS ?= -std=c++11 -O2
CFLAGS   ?= -O2
CPPFLAGS += -Iinclude

UNAME_S := $(shell uname -s 2>/dev/null)

ifeq ($(UNAME_S),Linux)
    EXEC   := a.out
    LDLIBS := -lglfw -lGL -ldl -lpthread
else ifeq ($(UNAME_S),Darwin)
    EXEC     := a.out
    BREW     := $(shell brew --prefix 2>/dev/null)
    CPPFLAGS += -I$(BREW)/include
    LDFLAGS  += -L$(BREW)/lib
    LDLIBS   := -lglfw -framework OpenGL
else
    # Windows: MSYS2/MinGW (uname = MINGW64_NT-..., MSYS_NT-...)
    EXEC   := a.exe
    LDLIBS := -lglfw3 -lopengl32 -lgdi32
endif

SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o) glad.o
HDR := $(wildcard *.h)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

%.o: %.cpp $(HDR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

glad.o: glad.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) a.out a.exe solution.txt

.PHONY: all clean
