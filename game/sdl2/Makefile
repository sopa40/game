SRC = sdl2.c
BIN = sdl2
CC = gcc
OBJS = $(SRC:.c=.o)
# Many more warnings, not all "good"
WARNINGS = \
         -Wall \
         -Wextra \
         -pedantic \
         -Wchkp \
         -Wmissing-include-dirs \
         -Wswitch-default \
         -Wunused-parameter \
         -Wuninitialized \
         -Wfloat-equal \
         -Wshadow \
         -Wunsafe-loop-optimizations \
         -Wbad-function-cast \
         -Wcast-align \
         -Wwrite-strings \
         -Wconversion \
         -Wlogical-op \
         -Wredundant-decls \
         -Wnested-externs \
         -Wstrict-prototypes \
         -Wmissing-prototypes \
         -Wstrict-aliasing \
         -Wold-style-definition \
         -Wnull-dereference \
         -Wduplicated-cond \
         -Werror \

LDFLAGS = -Wl,-rpath=/usr/lib
CFLAGS = $(WARNINGS) $(shell pkg-config --cflags sdl2 SDL2_image)
LDLIBS =  $(shell pkg-config --libs sdl2 SDL2_image)

all: $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

clean:
	-rm -f $(OBJS)
