CC = cc
CFLAGS = -Wall

TCPSERVER = tcpserver
TCPSERVERO = tcpserver.o log.o iofunc.o conf.o ini.o

default:
	$(MAKE) -r all

all:
	$(MAKE) -r $(TCPSERVER)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TCPSERVER): $(TCPSERVERO)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	-$(RM) *.o
	-$(RM) TCPSERVER

