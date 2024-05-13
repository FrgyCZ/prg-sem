#CC:=ccache $(CC)
CFLAGS+= -Wall -Werror -std=gnu99 -gdwarf-4
LDFLAGS=-lpthread

#HW=prgsem
BINARIES=prgsem

CFLAGS+=$(shell sdl2-config --cflags)
LDFLAGS+=$(shell sdl2-config --libs) -lSDL2_image

# Exclude module.c from compilation
MODULE_EXCLUDE=module.c

all: $(BINARIES)

# Compile all.c files except for those listed in MODULE_EXCLUDE
OBJS=$(patsubst %.c,%.o,$(filter-out $(MODULE_EXCLUDE),$(wildcard *.c)))

prgsem: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(BINARIES) $(OBJS)
