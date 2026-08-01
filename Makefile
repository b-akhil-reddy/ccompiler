OBJECTS=./build/compile_process.o ./build/compiler.o
INCLUDES= -I./

all: ${OBJECTS}
	gcc main.c ${INCLUDE} ${OBJECTS} -g -o ./main

./build/compiler.o: ./compiler.c
	gcc ./compiler.c ${INCLUDES} -o ./build/compiler.o -g

./build/compile_process.o: ./compile_process.c
	gcc ./compile_process.c ${INCLUDES} -o ./build/compile_process.o -g

clean:
	rm ./main
	rm -rf ${OBJECTS}