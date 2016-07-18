objects = main.o list.o

target = file_manager

CFLAGS = -Wall -g -pedantic

LDLIBS = -lncurses

all : $(target) reloc


$(target) : $(objects)
		$(CC) -o $(target) $(LDLIBS) $^

$(objects) : list.h
$(objects) : %.o : %.c
		$(CC) -c $(CFLAGS) $<

reloc :
		mkdir -p ../build
		mv $(target) ../build

.PHONY : clean

clean :
		-rm -r *.o ../build
