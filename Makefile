OBJECTS=./build/compile_process.o ./build/compiler.o ./build/helpers/vector.o ./build/helpers/buffer.o ./build/lex_process.o ./build/lexer.o
INCLUDES= -I./

all: clean ${OBJECTS}
	gcc main.c ${INCLUDE} ${OBJECTS} -g -o ./main

./build/compiler.o: ./compiler.c
	gcc ./compiler.c ${INCLUDES} -o ./build/compiler.o -g -c

./build/compile_process.o: ./compile_process.c
	gcc ./compile_process.c ${INCLUDES} -o ./build/compile_process.o -g -c

./build/lex_process.o: ./lex_process.c
	gcc ./lex_process.c ${INCLUDES} -o ./build/lex_process.o -g -c

./build/lexer.o: ./lexer.c
	gcc ./lexer.c ${INCLUDES} -o ./build/lexer.o -g -c

./build/helpers/vector.o: ./helpers/vector.c
	gcc ./helpers/vector.c ${INCLUDES} -o ./build/helpers/vector.o -g -c

./build/helpers/buffer.o: ./helpers/buffer.c
	gcc ./helpers/buffer.c ${INCLUDES} -o ./build/helpers/buffer.o -g -c

clean:
	rm ./main | exit 0
	rm -rf ${OBJECTS} | exit 0