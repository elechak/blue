


DLLS = dict.dll list.dll streams.dll time.dll xml.dll threads.dll sockets.dll random.dll __hash.dll __graphics.dll __sql.dll regex.dll

TARGETS = $(DLLS)


all: $(TARGETS)

clean:
	rm -f  $(TARGETS)
	rm -f  *.dll
	$(MAKE) -C graphics clean
	$(MAKE) -C sql clean
	$(MAKE) -C hash clean


sockets.dll         : sockets.c 
dict.dll            : dict.c 
streams.dll         : streams.c
list.dll            : list.c 
threads.dll         : threads.c 
	$(CC) $(CFLAGS) -o $@  -shared $^ -lpthread
time.dll: time.c
	$(CC) $(CFLAGS) -o $@  -shared $^ -lm
regex.dll: regex.c
	$(CC) $(CFLAGS) -o $@  -shared $^
    
__graphics.dll  :
	$(MAKE) -C graphics

__sql.dll  :
	$(MAKE) -C sql

__hash.dll  :
	$(MAKE) -C hash


%.dll : %.c
	$(CC) $(CFLAGS) -o $@  -shared -fPIC $^


