#!/usr/bin/make -f
CFLAGS:=-Wall -Wshadow -pedantic -g -O2

libcgi.a(cgi.o attr.o) : cgi.o attr.o

.PHONY : clean

clean : 
	$(RM) cgi.o attr.o libcgi.a
