CC=g++
CFLAGS=-D_REENTRANT -g -Wall -Wextra -DYYDEBUG=1 -fPIC
LFLAGS= -lpthread -ldl
OBJS=Main.o Sockets.o Threads.o HttpResponse.o Config.o
PLUGINS=basic_auth.so

server: http.tab.o lex.yy.o $(OBJS)
		$(CC) $(LFLAGS) $(CFLAGS) $(OBJS) http.tab.o lex.yy.o -o server

$(OBJS): %.o: %.cpp
		$(CC) $(CFLAGS) -c $< -o $@

http.tab.o: http.tab.c
		$(CC) $(CFLAGS) -c $< -o $@

lex.yy.o: lex.yy.cc
		$(CC) $(CFLAGS) -c $< -o $@

http.tab.c:
	bison parse/http.y

lex.yy.cc:
	flex parse/http.l

$(PLUGINS): %.so: %.cpp
		$(CC) $(CFLAGS) -fPIC -shared $< -o $@ Config.o HttpResponse.o
clean:
	rm -f http.tab.c
	rm -f lex.yy.cc
	rm -f http.tab.h
	rm -f location.hh position.hh stack.hh
	rm -f *.o
	rm -f *.so

