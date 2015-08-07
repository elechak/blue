

all:
	wine cmd /c build.bat

test:
	wine cmd /c test.bat


fast-install:
	cp blue.exe /home/erik/.wine/drive_c/Program\ Files/Blue/
	cp blue.ico /home/erik/.wine/drive_c/Program\ Files/Blue/
	cp /home/erik/.wine/drive_c/MinGW/bin/freetype6.dll  /home/erik/.wine/drive_c/Program\ Files/Blue/
	cp /home/erik/.wine/drive_c/MinGW/bin/libpng13.dll  /home/erik/.wine/drive_c/Program\ Files/Blue/
	cp /home/erik/.wine/drive_c/MinGW/bin/zlib1.dll  /home/erik/.wine/drive_c/Program\ Files/Blue/
	cp *.dll /home/erik/.wine/drive_c/Program\ Files/Blue/lib/
	cp -R ../demo/* /home/erik/.wine/drive_c/Program\ Files/Blue/demo/
	cp ../resource/* /home/erik/.wine/drive_c/Program\ Files/Blue/resource



clean:
	rm -f *.o
	rm -f *.dll
	rm -f *.exe
	rm -f *.a
	rm -f *.lib
	rm -f grammer.c
	rm -f grammer.h
	rm -f grammer.out
	rm -f grammer.y
	rm -f blue.implib
	rm -f lempar.c
	rm -rf graphics
	rm -rf installer

