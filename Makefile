NCURSES_CFLAGS := $(shell pkg-config --cflags ncursesw)
NCURSES_LIBS := $(shell pkg-config --libs ncursesw)
LIBS += $(NCURSES_LIBS)
CFLAGS += $(NCURSES_CFLAGS)
SRCS = main.c
OBJS = $(SRCS:.c=.o)
all: fsm
fsm: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o fsm $(LIBS)
%.o: %.c
	gcc $(CFLAGS) -c $< -o $@
clean:
	rm -f $(OBJS) fsm
