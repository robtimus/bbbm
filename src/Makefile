CC          = gcc
BBBM_CFLAGS = $(CFLAGS) -Wall -O2 `pkg-config --cflags gtk+-2.0`
BBBM_LIBS   = $(LIBS) `pkg-config --libs gtk+-2.0`
OBJS        = util.o options.o image.o dialogs.o bbbm.o main.o
BINDIR      = /usr/local/bin

bbbm: $(OBJS)
	$(CC) $(BBBM_CFLAGS) $(BBBM_LIBS) $(OBJS) -o bbbm

.c.o:
	$(CC) $(BBBM_CFLAGS) -c $< -o $@

install:
	cp bbbm $(BINDIR);

uninstall:
	rm -f $(BINDIR)/bbbm

clean:
	rm -f *.o bbbm
