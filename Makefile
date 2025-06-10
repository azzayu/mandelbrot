

all:mandelbrot

mandelbrot: mandelbrot.c
	gcc mandelbrot.c -o mandelbrot -Ofast

clean: 
	rm mandelbrot
