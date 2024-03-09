
CC	:= gcc
CFLAGS := -g -Wall

TARGETS :=  libtsl.a  app get

all: $(TARGETS)

TSL_SRC :=  tsl.c
TSL_OBJS := $(TSL_SRC:.c=.o)

libtsl.a: $(TSL_OBJS)
	ar rcs $@ $(TSL_OBJS)

TSL_LIB :=  -L . -l tsl

tsl.o: tsl.c tsl.h
	gcc -c $(CFLAGS) -m32 -o $@ tsl.c

app.o: app.c  tsl.h
	gcc -c $(CFLAGS) -m32  -o $@ app.c

app: app.o libtsl.a
	gcc $(CFLAGS) -m32  -o $@ app.o  $(TSL_LIB)

get: get.c
	gcc $(CFLAGS) -m32 -o $@ $<

clean:
	rm -rf core  *.o $(TARGETS)
	
