

all:mandelbrot

mandelbrot: mandelbrot.c
	gcc mandelbrot.c -o mandelbrot

clean: 
	rm mandelbrot