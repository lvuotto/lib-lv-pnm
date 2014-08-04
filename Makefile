
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic
targets = lv-pnm.o

all: $(targets)


lv-pnm.o: lv-pnm.c lv-pnm.h lv-pnm-types.h
	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	rm -f $(targets)
