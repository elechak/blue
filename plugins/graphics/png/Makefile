




OBJS = png.o
DLLS = ../../png.dll
TARGETS = $(DLLS)

all: $(TARGETS)

clean:
	rm -f  $(TARGETS)
	rm -f  *.o


../../png.dll     :  $(OBJS)
	$(CC) $(CFLAGS) -o $@  -shared -fPIC $^ -lpng -lz -lm


%.o : %.c
	$(CC) $(CFLAGS) -c -fPIC $<


