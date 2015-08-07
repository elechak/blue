
CC=gcc
export CC

PACKAGEDIR=$(shell pwd |sed 's/\/.*\///')

ifndef prefix
prefix=/opt
endif

INSTALL_DIR= $(prefix)/blue

CFLAGS= -Wall -O3 -DNDEBUG -DREENTRANT -fPIC
#CFLAGS= -Wall -g -DREENTRANT -fPIC
export CFLAGS

all: blue plugins__

plugins__:
	$(MAKE) -C plugins

fast:
	$(MAKE) all
	$(MAKE) uninstall
	$(MAKE) install
	$(MAKE) test

xx:
	$(MAKE) all
	$(MAKE) uninstall
	$(MAKE) install

install:  install_link

install_script:
	mkdir -p $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)/lib/
	cp $(DLLS) $(INSTALL_DIR)/lib/
	mkdir -p $(INSTALL_DIR)/module/
	cp $(DLLS) $(INSTALL_DIR)/module/
	cp blue $(INSTALL_DIR)/
	echo "#! /usr/bin/bash" >/usr/local/bin/blue
	echo "export BLUE=$(INSTALL_DIR)" >>/usr/local/bin/blue
	echo "$(INSTALL_DIR)/blue \$$@" >>/usr/local/bin/blue
	chmod 777 /usr/local/bin/blue

install_link:
	mkdir -p $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)/lib/
	mkdir -p $(INSTALL_DIR)/module/
	mkdir -p $(INSTALL_DIR)/resource/
	mkdir -p $(INSTALL_DIR)/build/
	cp blue $(INSTALL_DIR)/
	cp plugins/*.dll $(INSTALL_DIR)/lib/
	cp module/* $(INSTALL_DIR)/module/
	cp resource/* $(INSTALL_DIR)/resource/
	cp *.h $(INSTALL_DIR)/build/
	ln -s $(INSTALL_DIR)/blue /usr/local/bin/blue

uninstall   :
	rm -rf $(INSTALL_DIR)
	rm -f /usr/local/bin/blue

tgz   :
	$(MAKE) clean
	cd ..; tar --exclude "_FOSSIL_" --exclude "fossil-blue" --exclude "manifest" --exclude "manifest.uuid" --exclude "blue.tgz" -cvzf $(PACKAGEDIR)/blue.tgz $(PACKAGEDIR)

7z        :
	$(MAKE) clean
	tar -cvf blue.tar .
	7zr a blue.tar.7z blue.tar
	rm blue.tar

COMP= assembler.o compiler.o
ALGS= bstring.o splaytree.o bytes.o dynlib.o stream.o files.o threading.o  index.o skiparray.o
OBJ=  blue.o str.o number.o bcode.o codeblock.o array.o interp.o native.o link.o dictionary.o

bstring.o       : bstring.c bstring.h
splaytree.o     : splaytree.c splaytree.h comparison.h
bytes.o         : bytes.c bytes.h
dynlib.o        : dynlib.c dynlib.h
stream.o        : stream.c stream.h
files.o         : files.c files.h
threading.o     : threading.c threading.h
index.o         : index.c index.h
skiparray.o     : skiparray.c skiparray.h

lemon: lemon.c
	$(CC) $(CFLAGS) -o $@  $^ 

grammer.c: lemon grammer.y
	./lemon grammer.y    

compiler.o: compiler.c grammer.c
assembler.o: assembler.c assembler.h

str.o          :  str.c str.h 
number.o       :  number.c number.h
bcode.o        :  bcode.c bcode.h
codeblock.o    :  codeblock.c codeblock.h
array.o        :  array.c array.h
native.o       :  native.c native.h
interp.o       :  interp.c interp.h
link.o         :  link.c link.h typedefs.h
dictionary.o   :  dictionary.c dictionary.h

blue: main.c global.h $(OBJ) $(COMP) $(ALGS) 
	${CC} ${CFLAGS} -o $@ -ldl -rdynamic -lpthread -lm main.c $(OBJ) $(COMP) $(ALGS) 

test:
	$(MAKE) -C testing test

clean:
	$(MAKE) -C testing clean
	$(MAKE) -C plugins clean
	$(MAKE) -C win32 clean
	rm -f  *.o
	rm -f  *.lib
	rm -f  *.dll
	rm -f  blue
	rm -f  blue*.tgz
	rm -f  blue*.bz2
	rm -f  blue*.tar
	rm -f  blue*.7z
	rm -f  blue*.exe
	rm -f  cachegrind.out.*
	rm -f  callgrind.out.*
	rm -f  massif.*
	rm -f  gmon.out
	rm -f  benchmark/gmon.out
	rm -f  demo/gmon.out
	rm -f  tests/gmon.out
	rm -f  tests2/gmon.out
	rm -rf grammer.c
	rm -rf grammer.h
	rm -rf grammer.out
	rm -rf compiler.o
	rm -rf compiler.lib
	rm -rf assembler.o
	rm -rf lemon    
