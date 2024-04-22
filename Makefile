NCURSES_CFLAGS := $(shell pkg-config --cflags ncursesw)
NCURSES_LIBS := $(shell pkg-config --libs ncursesw)
LIBS += $(NCURSES_LIBS)
CFLAGS += $(NCURSES_CFLAGS)
SRCS = main.c
OBJS = $(SRCS:.c=.o)
TARGET = FSUtility
all: $(TARGET)
$(TARGET): $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)
%.o: %.c
	gcc $(CFLAGS) -c $< -o $@
clean:
	rm -f $(OBJS) $(TARGET)
run:
	sudo ./$(TARGET)
