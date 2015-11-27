UNAME := $(shell uname)
CC=clang
SOURCES=src/window.o
MODULE=Window.so

ifeq ($(UNAME), Linux)
SOFLAGS=-shared
CFLAGS+=-Wall -Isrc/ -I./ -I/usr/include/lua5.2/ -D _BSD_SOURCE -fPIC
LDFLAGS+=-L./ -L/usr/local/lib -llua5.2 -lpthread
endif
ifeq ($(UNAME), Darwin)
SOFLAGS=-bundle -undefined dynamic_lookup
CFLAGS+=-Wall -Isrc/ -I./ -I/usr/local/include/ -D _BSD_SOURCE -fPIC -F/Library/Frameworks 
LDFLAGS+=-L./ -L/usr/local/lib -llua -lpthread -F/Library/Frameworks -framework SDL2
endif

all: clean build

build: $(SOURCES)
	$(CC) -g $(CFLAGS) $(SOFLAGS) -o $(MODULE) $^ $(LDFLAGS)

clean:
	rm -f $(MODULE) *.so *.o
