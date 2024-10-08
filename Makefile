build: clean
	gcc -g -Wall -std=c99 ./src/*.c -lSDL2 -lm -o renderer
fast:
	gcc -std=c99 -O3 ./src/*.c -lSDL2 -lm -o renderer_fast
run: build
	./renderer
clean:
	rm -f ./renderer
