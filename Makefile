CC = gcc
CFLAGS = --std=c99 -Wall -Wextra -DGL_DEBUG

OUT  = neko
OBJ  = \
	src/main.o     \
	src/math.o     \
	src/renderer.o 

ifeq ($(OS),Windows_NT)
    LDFLAGS = -lgdi32 -lopengl32
	OBJ += src/win32.o
else
	LDFLAGS = -lGL -lGLU -lX11
	OBJ += src/x11.o
endif

build: $(OBJ)
	$(CC) -o $(OUT) $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f $(OBJ) $(OUT)
