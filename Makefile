APPS = alarm2server
OBJS = main.o cJSON.o
CC= arm-hisiv100nptl-linux-gcc
CFLAGS = -Wall -O -g -lm -lpthread

main: $(OBJS)
	$(CC) $(OBJS) -o $(APPS) -lm -lpthread

main.o: main.c cJSON.h
	$(CC) $(CFLAGS) -c main.c -o main.o

cJSON.o: cJSON.c cJSON.h
	$(CC) $(CFLAGS) -c cJSON.c -o cJSON.o

clean:
	rm -f *.o
