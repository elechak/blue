

OBJS = stdshapes.o
DLLS = ../../stdshapes.dll
TARGETS = $(DLLS)

all: $(TARGETS)

clean:
	rm -f  $(TARGETS)
	rm -f  *.o


../../stdshapes.dll  :  $(OBJS)
	$(CC) $(CFLAGS) -o $@  -shared -fPIC $^ -lm


%.o : %.c
	$(CC) $(CFLAGS) -c -fPIC $<


