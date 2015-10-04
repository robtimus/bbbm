CC     = gcc
CFLAGS = -Wall -O2 `pkg-config --cflags gtk+-2.0`
LIBS   = `pkg-config --libs gtk+-2.0`
OBJS = util.o dialogs.o bbbm.o image.o options.o main.o

bbbm: $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) -o bbbm

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

install:
	cp bbbm /usr/local/bin

uninstall:
	rm -f /usr/local/bbbm

clean:
	rm -f *.o bbbm
