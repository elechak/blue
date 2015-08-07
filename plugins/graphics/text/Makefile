




OBJS = text.o
DLLS = ../../text.dll
TARGETS = $(DLLS)

all: $(TARGETS)

clean:
	rm -f  $(TARGETS)
	rm -f  *.o


../../text.dll     :  $(OBJS)
	$(CC) $(CFLAGS) -o $@  -shared -fPIC $^ -lfreetype 


%.o : %.c
	$(CC) $(CFLAGS) -c -fPIC $< -I /usr/include/freetype2


