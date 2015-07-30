CC = cc
CFLAGS = -Wall

TCPSERVER = tcpserver
TCPSERVERO = tcpserver.o log.o iofunc.o conf.o ini.o http.o map.o http_parser.o string.o

default:
	$(MAKE) -r all

all:
	$(MAKE) -r $(TCPSERVER)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TCPSERVER): $(TCPSERVERO)
	$(CC) $(CFLAGS) $^ -o $@

debug:
	$(CC) $(CFLAGS) -g -O0 tcpserver.c log.c iofunc.c conf.c ini.c http.c map.c http_parser.c string.c -o $(TCPSERVER)

clean:
	-$(RM) *.o
	-$(RM) $(TCPSERVER)
	-$(RM) -r $(TCPSERVER).dSYM

