
SHARED="-dynamiclib"

CC="gcc -O3 -Wall -fPIC -DREENTRANT -DNDEBUG "

${CC}  -c bstring.c 
${CC}  -c dynlib.c 
${CC}  -c splaytree.c 
${CC}  -c skiparray.c 
${CC}  -c bytes.c 
${CC}  -c stream.c 
${CC}  -c files.c 
${CC}  -c threading.c 
${CC}  -c index.c
${CC} -c str.c 
${CC} -c number.c 
${CC} -c codeblock.c 
${CC} -c bcode.c 
${CC} -c array.c 
${CC} -c native.c 
${CC} -c interp.c 
${CC} -c link.c 
${CC} -c dictionary.c 
${CC} -c blue.c

# GRAMMER
${CC} -o lemon lemon.c
./lemon grammer.y
${CC} -c compiler.c -I.
${CC} -c assembler.c -I.


COMP="assembler.o compiler.o"
ALGS="bstring.o splaytree.o bytes.o dynlib.o stream.o files.o threading.o  index.o skiparray.o"
OBJ="blue.o str.o number.o bcode.o codeblock.o array.o interp.o native.o link.o dictionary.o"

${CC} -o blue.exe  main.c ${COMP} ${ALGS} ${OBJ} -ldl -lm -lpthread

${CC} ${SHARED} -fPIC -o dict.dll  plugins/dict.c 


