CC = clang++
LIBS = -lSDL2
BINDIR = bin
BIN = tetris
SRC = sdlmain.cpp core.cpp
CFLAGS = -std=c++17 -fno-rtti -Wall -Wextra -Wpedantic -Wshadow

ifeq ($(OS),Windows_NT)
	LIBS := -lmingw32 -lSDL2main $(LIBS) -lgdi32
	BIN := $(BIN).exe
endif

debug:
	${CC} ${CFLAGS} -g -o ${BINDIR}/${BIN} ${SRC} ${LIBS}

release:
	${CC} ${CFLAGS} -O2 -o ${BINDIR}/${BIN} ${SRC} ${LIBS}

run: debug
	${BINDIR}/${BIN}

clean:
	rm ${BINDIR}/${BIN}

syntax:
	${CC} ${CFLAGS} -fsyntax-only ${SRC}
