SRC := $(wildcard ./*/*.cpp) encode.cpp decode.cpp main.cpp
CPPFLAGS := -Wno-write-strings 

CC = g++
LD = ld

OBJ := $(wildcard */*.o) encode.o decode.o main.o

build: ${OBJ}
	${CC} ${LDFLAGS} ${CPPFLAGS} ${OBJ} -o fv1_transcode

clean:
	rm ${OBJ} fv1_transcode

cleanobj:
	rm ${OBJ}


%.o : %.cpp
	$(CC) ${LDFLAGS} $(CPPFLAGS) -c $< -o $@

run: build
	./fv1_transcode

static: ${OBJ}
	${CC} ${LDFLAGS} ${CPPFLAGS} ${OBJ} -o fv1_transcode -static
