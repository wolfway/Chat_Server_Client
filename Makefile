OBJS=messserver.o messclient.o 


CC=gcc
CFLAGS= -g -O -Wall -ansi

#Linux, Solaris
#PTHREAD_COMPILE=$(CC) $(CFLAGS) -lpthread  -lrt -o $@ $^

#FreeBSD
#PTHREAD_COMPILE=$(CC) $(CFLAGS) -pthread -o $@ $^

#PTHREAD=if [ `uname` = 'SunOS' ]; then THREAD='-lnet -lpthread  -lrt';else THREAD='-pthread -net';fi
PTHREAD=if [ `uname` = 'Linux' ]; then THREAD='./libnet_Linux.a -lpthread  -lrt';else THREAD='-pthread ./libnet_BSD.a';fi
PTHREAD_LINK=$(PTHREAD); $(CC) $(CFLAGS) -o $@ 

SERVER_OBJS=messserver.o

CLIENT_OBJS=messclient.o

TEST_OBJS=test.o

all: messserver messclient test 

messserver: $(SERVER_OBJS)
	$(PTHREAD_LINK) $(SERVER_OBJS) $$THREAD

messclient: $(CLIENT_OBJS)
	$(PTHREAD_LINK) $(CLIENT_OBJS) $$THREAD

test:  test.c 
	$(CC) $(CFLAGS) test.c -o test 

clean:
	rm -f *.o messclient messserver test *.core

debug:
	#echo $(THREAD)
	#echo $(PTHREAD)
	#echo $(PTHREAD_LINK)
	#echo $(PTHREAD_COMPILE)
