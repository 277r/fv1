SRC := $(wildcard ./*/*.cpp) $(wildcard ./*.cpp)
OBJ := $(SRC:.cpp=.o)

CPPFLAGS := -Wno-write-strings 
LDFLAGS := -lpthread -lavformat -lavcodec -lavutil 

CC = g++
LD = ld



build: ${OBJ}
	${CC} ${CPPFLAGS} ${OBJ} -o fv1_transcode ${LDFLAGS} 

clean:
	rm ${OBJ} fv1_transcode

cleanobj:
	rm ${OBJ}


%.o : %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

run: build
	./fv1_transcode

static: ${OBJ}
	${CC} ${LDFLAGS} ${CPPFLAGS} ${OBJ} -o fv1_transcode -static
