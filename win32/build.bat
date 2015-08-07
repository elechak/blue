echo on

set PATH=%PATH%;c:\MinGW\bin
set CC=gcc -O3 -Wall -DREENTRANT -DNDEBUG -DBUILD_DLL
REM set CC=gcc -g -Wall -DREENTRANT -DNDEBUG 

%CC%  -c ../bstring.c 
%CC%  -c ../dynlib.c 
%CC%  -c ../splaytree.c 
%CC%  -c ../skiparray.c 
%CC%  -c ../directory.c 
%CC%  -c ../bytes.c 
%CC%  -c ../stream.c 
%CC%  -c ../files.c 
%CC%  -c ../threading.c 
%CC%  -c ../index.c

%CC% -c ../str.c 
%CC% -c ../number.c 
%CC% -c ../codeblock.c 
%CC% -c ../bcode.c 
%CC% -c ../array.c 
%CC% -c ../native.c 
%CC% -c ../interp.c 
%CC% -c ../link.c 
%CC% -c ../dictionary.c 
%CC% -c ../blue.c

REM compiler 
%CC% -o lemon ../lemon.c
copy ../grammer.y .
copy ../lempar.c .
lemon grammer.y
%CC% -c ../compiler.c -I. -I..
%CC% -c ../assembler.c -I. -I..


set COMP=assembler.o compiler.o
set ALGS=bstring.o splaytree.o bytes.o dynlib.o stream.o files.o threading.o  index.o skiparray.o
set OBJ=blue.o str.o number.o bcode.o codeblock.o array.o interp.o native.o link.o dictionary.o

REM build blue
%CC%  -o blue.exe  ../main.c %COMP% %ALGS% %OBJ% -Wl,--export-all-symbols,--out-implib,blue_implib.a -lwsock32
echo off




REM PLUGINS
set CCPL=%CC% -shared ../plugins/
%CCPL%dict.c -o dict.dll  blue_implib.a
%CCPL%list.c -o list.dll  blue_implib.a
%CCPL%streams.c -o streams.dll  blue_implib.a -lwsock32
%CCPL%threads.c -o threads.dll  blue_implib.a
%CCPL%xml.c -o xml.dll  blue_implib.a
%CCPL%random.c -o random.dll  blue_implib.a
%CCPL%time.c -o time.dll  blue_implib.a -lm -lregex
%CCPL%regex.c -o regex.dll  blue_implib.a -lregex 



REM SQL
set CC=gcc -O3 -DREENTRANT -DNDEBUG -DBUILD_DLL
%CC% -I../plugins/sql/sqlite/ -c ../plugins/sql/sqlite/sqlite3.c
set CC=gcc -Wall -O3 -DREENTRANT -DNDEBUG -DBUILD_DLL -shared
%CC% -I../plugins/sql/sqlite/ -o sql.dll ../plugins/sql/sql.c sqlite3.o blue_implib.a


REM HASH
set CC=gcc -O3 -DREENTRANT -DNDEBUG -DBUILD_DLL
%CC% -I../plugins/hash/ -c ../plugins/hash/md5.c
set CC=gcc -Wall -O3 -DREENTRANT -DNDEBUG -DBUILD_DLL -shared
%CC% -I../plugins/hash/ -o hash.dll ../plugins/hash/hash.c md5.o blue_implib.a


REM GRAPHICS
set CC=gcc -O3 -Wall -DREENTRANT -DNDEBUG -DBUILD_DLL -c ../plugins/graphics/
%CC%lists.c
%CC%window.c
%CC%view.c
%CC%layer.c
%CC%light.c
%CC%shape.c
%CC%image.c
%CC%childman.c
%CC%glmat.c
%CC%graphics_win32.c

set CC=gcc -O3 -Wall -DREENTRANT -DNDEBUG -DBUILD_DLL -shared
%CC% -o graphics.dll lists.o glmat.o window.o view.o layer.o shape.o light.o image.o childman.o graphics_win32.o blue_implib.a -lglut32 -lglu32 -lopengl32 -lm -lgdi32 -lwsock32

set CC=gcc -O3 -Wall -DREENTRANT -DNDEBUG -DBUILD_DLL  -shared
%CC% -o png.dll ../plugins/graphics/png/png.c blue_implib.a graphics.dll -lm -lz -lpng

set CC=gcc -O3 -Wall -DREENTRANT -DNDEBUG -DBUILD_DLL -I "c:\MinGW\include\freetype2" -shared
%CC% -o text.dll ../plugins/graphics/text/text.c blue_implib.a graphics.dll -lm -lz -lpng  -lopengl32 -lfreetype

set CC=gcc -O3 -Wall -DREENTRANT -DNDEBUG -DBUILD_DLL -shared
%CC% -o stdshapes.dll ../plugins/graphics/stdshapes/stdshapes.c blue_implib.a graphics.dll -lopengl32 





















