

../hash.dll  : hash.c md5.o
	$(CC) $(CFLAGS) -o $@  -shared -fPIC $^

md5.o: md5.c md5.h
	$(CC)  -O3 -DNDEBUG -DREENTRANT -fPIC -c md5.c 

clean:
	rm -f md5.o




