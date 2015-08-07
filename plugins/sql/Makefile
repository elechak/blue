

../sql.dll  : sql.c sqlite3.o
	$(CC) $(CFLAGS) -o $@  -shared -fPIC $^

sqlite3.o: sqlite/sqlite3.c sqlite/sqlite3.h
	$(CC) -Isqlite  -O3 -DNDEBUG -DREENTRANT -fPIC -c sqlite/sqlite3.c 

clean:
	rm -f sqlite3.o




