CC          = gcc
BBBM_CFLAGS = $(CFLAGS) -Wall -O2 `pkg-config --cflags gtk+-2.0`
BBBM_LIBS   = $(LIBS) `pkg-config --libs gtk+-2.0`
BINDIR      = /usr/local/bin

bbbm: $(OBJS)
	$(CC) $(BBBM_CFLAGS) *.c $(BBBM_LIBS) -o bbbm

install:
	cp bbbm $(BINDIR)

uninstall:
	rm -f $(BINDIR)/bbbm

clean:
	rm -f bbbm
