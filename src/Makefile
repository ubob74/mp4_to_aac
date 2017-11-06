.PHONY := clean

CPP := gcc
BIN := mp4_to_aac
OBJ := main.o trak.o box.o utils.o stb.o adts.o

all: $(BIN)
	test -d ../bin && mv $(BIN) ../bin

$(BIN): $(OBJ)
	$(CPP) $^ -Wall -O0 -o $@

.c.o:
	$(CPP) -DDEBUG -I./ -I../include -g -c $<

clean:
	rm -f *~ *.o ../bin/$(BIN)

