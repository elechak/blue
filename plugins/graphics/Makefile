

OBJS = lists.o glmat.o graphics.o childman.o window.o view.o layer.o shape.o image.o light.o

DLLS = ../graphics.dll ../png.dll ../text.dll ../stdshapes.dll

TARGETS = $(DLLS)


all: $(TARGETS)

clean:
	rm -f  $(TARGETS)
	rm -f  *.o
	$(MAKE) -C png clean
	$(MAKE) -C text clean
	$(MAKE) -C stdshapes clean


lists.o: lists.c lists.h
glmat.o: glmat.c glmat.h
graphics.o: graphics.c graphics.h
childman.o: childman.c graphics.h
window.o: window.c graphics.h
view.o: view.c graphics.h
layer.o: layer.c graphics.h glmat.o
shape.o: shape.c graphics.h glmat.o
image.o: image.c graphics.h
light.o: light.c graphics.h

../graphics.dll     :  $(OBJS) 
	$(CC) $(CFLAGS) -o $@ -shared -fPIC $^ -L/usr/X11R6/lib -lGL -lGLU -lpthread -lm -lXxf86vm

../png.dll: png/png.c
	$(MAKE) -k -C png

../text.dll: text/text.c
	$(MAKE) -k -C text

../stdshapes.dll: stdshapes/stdshapes.c
	$(MAKE) -k -C stdshapes


%.o : %.c
	$(CC) $(CFLAGS) -c -fPIC $<


