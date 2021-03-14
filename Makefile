CC := clang
CFLAGS := -g -Wall -Werror

all: libetsbefriends.so segfault-test lazycopy-test

clean:
	rm -rf libetsbefriends.so lazycopy-test segfault-test

libetsbefriends.so: libetsbefriends.c
	$(CC) $(CFLAGS) -shared -fPIC -o libetsbefriends.so libetsbefriends.c

segfault-test: segfault-test.c libetsbefriends.so
	$(CC) $(CFLAGS) -o segfault-test segfault-test.c -L. -letsbefriends

lazycopy-test: lazycopy-test.c lazycopy.c lazycopy.h
	$(CC) $(CFLAGS) -o lazycopy-test lazycopy-test.c lazycopy.c
