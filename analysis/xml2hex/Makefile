CC = gcc
C_FLAGS = -O `pkg-config --cflags glib-2.0`
O_FILES = main.o
PROGNAME = getpkt
L_FLAGS = `pkg-config --libs glib-2.0`

all: $(PROGNAME)
	$(CC) $(C_FLAGS) $(L_FLAGS) main.c -o $(PROGNAME)
#	cp $(PROGNAME) /tmp

exec:
	/tmp/$(PROGNAME)

clean:
	$(RM) $(O_FILES)
	$(RM) *.bak
	$(RM) *~
