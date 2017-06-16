CC = g++

CFLAGS = -Wall -Wextra -ggdb -fpermissive

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
LDFLAGS = -lGL -lGLEW -lglut -lpng -lavcodec -lavformat -lavutil -lswscale -lstdc++
else
LDFLAGS = -lopengl32 -lglew32 -lfreeglut -lpng -lavcodec -lavformat -lavutil -lswscale -lstdc++
endif

SOURCES = glslrender.cpp encoder.cpp shaders.cpp
TARGET = frag2vid

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
