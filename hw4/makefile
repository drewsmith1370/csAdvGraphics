# Example 07
EXE=ex07

# Main target
all: $(EXE) vert.spv frag.spv

#  Msys/MinGW
ifeq "$(OS)" "Windows_NT"
CFLG=-O3 -Wall -DUSEGLEW
LIBS=-lglfw3 -lvulkan-1 -lm
CLEAN=rm -f *.exe *.o *.a
else
#  OSX
ifeq "$(shell uname)" "Darwin"
CFLG=-O3 -Wall -Wno-deprecated-declarations  -DUSEGLEW
LIBS=-lglfw -lvulkan -lm
#  Linux/Unix/Solaris
else
CFLG=-O3 -Wall
LIBS=-lglfw -lvulkan -lm
endif
#  OSX/Linux/Unix/Solaris
CLEAN=rm -f $(EXE) *.o *.a
endif

# Dependencies
ex07.o: ex07.c mat4.h
mat4.o: mat4.c mat4.h

# Compile rules
.c.o:
	gcc -c $(CFLG)  $<
.cpp.o:
	g++ -c $(CFLG)  $<

#  Compile shaders
vert.spv:vulkan.vert
	glslangValidator -V -o vert.spv vulkan.vert
frag.spv:vulkan.frag
	glslangValidator -V -o frag.spv vulkan.frag
#  Link
ex07:ex07.o mat4.o 
	gcc $(CFLG) -o $@ $^  $(LIBS)

#  Clean
clean:
	$(CLEAN)
