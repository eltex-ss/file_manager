objects = main.o list.o

target = file_manager

INCLUDE_FLAGS = -Iinclude

CFLAGS = -Wall -g -pedantic $(INCLUDE_FLAGS)

LDLIBS = -lncurses -pthread

VPATH = include

all : $(target) reloc


$(target) : $(objects)
		$(CC) -o $(target) $(LDLIBS) $^

$(objects) : list.h
$(objects) : %.o : %.c
		$(CC) -c $(CFLAGS) $< -o $@

reloc :
		mkdir -p ../build
		mv $(target) ../build

.PHONY : clean

clean :
		-rm -r *.o ../build
