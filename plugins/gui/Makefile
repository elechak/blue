CC=gcc -Wall -g

OBJS = text.o itemmanager.o lists.o glmat.o graphics.o window.o view.o layer.o shape.o image.o light.o 

gui.dll     :  $(OBJS) ../../blue.lib
	$(CC) $(CFLAGS) -o $@ -shared -fPIC $^  -L/usr/X11R6/lib -lGL -lGLU -lpthread -lm -lXxf86vm -lpng -lz -lm -lfreetype 

itemmanager.o: itemmanager.c graphics.h
lists.o: lists.c lists.h
glmat.o: glmat.c glmat.h
graphics.o: graphics.c graphics.h
window.o: window.c graphics.h
view.o: view.c graphics.h
layer.o: layer.c graphics.h glmat.o
shape.o: shape.c graphics.h glmat.o
image.o: image.c graphics.h
light.o: light.c graphics.h

text.o : text.c  graphics.h
	$(CC) $(CFLAGS) -c -fPIC -I /usr/include/freetype2 $<

clean:
	rm -f  *.o
	rm -f  *.dll

%.o : %.c
	$(CC) $(CFLAGS) -c -fPIC $<


