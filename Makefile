build: clean
	gcc -g -Wall -std=c99 ./src/*.c -lSDL2 -lm -o renderer
fast: clean
	gcc -std=c99 -O3 ./src/*.c -lSDL2 -lm -o renderer_fast
	./renderer_fast
run: build
	./renderer
clean:
	rm -f ./renderer*
