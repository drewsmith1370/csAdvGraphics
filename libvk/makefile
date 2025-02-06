# Example 07
EXE=driver

# Main target
all: $(EXE)

#  Msys/MinGW
ifeq "$(OS)" "Windows_NT"
CFLG=-O3 -Wall -DUSEGLEW
LIBS=-lglfw3 -lvulkan-1 -lm
CLEAN=rm -f *.exe *.o *.a *.spv
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
CLEAN=rm -f $(EXE) *.o *.a *.spv
endif

# Dependencies
driver.o: driver.c CSCIx239Vk.h
fatal.o: fatal.c CSCIx239Vk.h
vkBuffer.o: vkBuffer.c vkBuffer.h CSCIx239Vk.h
vkDevice.o: vkDevice.c vkDevice.h CSCIx239Vk.h
initwin.o: initwin.c CSCIx239Vk.h
mat4.o: mat4.c mat4.h

# Compile rules
.c.o:
	gcc -c $(CFLG) $<
.cpp.o:
	g++ -c $(CFLG) $<

#  Link
driver:driver.o fatal.o vkBuffer.o vkDevice.o initwin.o mat4.o
	gcc $(CFLG) -o $@ $^  $(LIBS)

#  Clean
clean:
	$(CLEAN)
