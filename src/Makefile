CPP  = g++
CC   = g++

INCLUDE =
LIBS =

SOURCE = $(wildcard *.cpp)
OBJ    = $(patsubst %.cpp,%.o,$(SOURCE))
BIN    = Tongyi

CXXFLAGS =
CFLAGS   = -Wall -g

RM = rm -f


all:	$(BIN)

%.o:	%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(BIN): $(OBJ)
	$(CPP) $(CXXFLAGS) $(LIBS) -o $@ $^

clean:
	${RM} $(OBJ) $(BIN)

.PHONY: all clean
