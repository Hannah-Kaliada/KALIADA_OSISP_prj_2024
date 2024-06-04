NCURSES_CFLAGS := $(shell pkg-config --cflags ncursesw)
NCURSES_LIBS := $(shell pkg-config --libs ncursesw)
LIBS += $(NCURSES_LIBS)
CFLAGS += $(NCURSES_CFLAGS) -Wno-macro-redefined

SRCS = main.c game.c gui.c operations.c find.c dir_size.c crc.c
OBJS = $(SRCS:.c=.o)
TARGET = FSUtility

all: $(TARGET)

$(TARGET): $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

sudo: clean all
	sudo ./$(TARGET)

run: clean all
	./$(TARGET)
