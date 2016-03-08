
all: a

TARGET = -mmacosx-version-min=10.8
LIBS = -F/System/Library/Frameworks -framework OpenGL -framework GLUT

a: a3.c
	clang a3.c -o a3 $(LIBS) $(TARGET)

scene: scene.c
	clang scene.c -o scene $(LIBS) $(TARGET)

ubuntu: a3.c
	gcc -o a3 a3.c -lglut -lGLU -lGL -lXmu -lXext -lX11 -lm

run:
	./a3