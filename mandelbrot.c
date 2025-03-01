
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define RED 0
#define GREEN 1
#define BLUE 2

typedef struct complex_s{
    double real;
    double im;
}complex;


void affiche_z(complex z){
    printf("%lf + %lfi\n", z.real, z.im);
}


complex iter_f(complex zn, complex z0){
    complex result;
    result.real = zn.real * zn.real - zn.im * zn.im + z0.real;
    result.im = 2 * zn.real * zn.im + z0.im;
    return result;
}

double module(complex z){
    return z.real * z.real + z.im * z.im;
}


complex translates_int_to_complex(int position, double x_min, double x_max, double y_min, double y_max, int x_size, int y_size){
    int x_pos = position%x_size;
    int y_pos =(int) floor(position/x_size);

    //printf("x : %i y : %i \n",x_pos, y_pos);

    double x_pos_d = (double) x_pos;
    double y_pos_d = (double) y_pos;
    double size_x_d = (double) x_size;
    double size_y_d = (double) y_size;

    complex value;
    value.real = (double) (x_pos_d/size_x_d) * (x_max - x_min) + x_min;
    value.im = (double) (y_pos_d/size_y_d) * (y_max - y_min) + y_min;

    return value;
}


int diverge(complex z0, int iter){
    complex zn = z0;
    int n = 0;
    while (n < iter){
        zn = iter_f(zn, z0);
        n++;
        if (module(zn) > 4){
            return n;
        }
    }
    return 0;
}


uint32_t time_to_colour(int time_to_div, uint8_t **colours, int length){
    int time = time_to_div % length;
    //printf("%i \n", time_to_div);
    //printf("%i %i %i \n", colours[time_to_div][RED], colours[time_to_div][GREEN], colours[time_to_div][BLUE]);
    int colour = colours[time][RED] + (colours[time][GREEN] << 8) + (colours[time][BLUE] << 16);
    if (time_to_div == 0){
        return 0;
    }
    return colour;
}


int length(char *s){
    int l = 0;
    while (s[l] != '\0'){
        l++;
    }
    return l;
}


uint8_t **get_colours(FILE *config, int *steps, int *nb_cols){
    *nb_cols = 1;
    char colours[1024];
    fgets(colours, sizeof colours, config);
    fgets(colours, sizeof colours, config);
    int i = 0;
    while (colours[i] != '\n'){
        if (colours[i] == ' '){
            (*nb_cols)++;
        }
        i++;
    }

    uint8_t **colour = malloc((*nb_cols) * sizeof(uint8_t*));

    for (int i = 0; i < *nb_cols ; i++){
        colour[i] = malloc(sizeof(uint8_t) * 3);
        char col[8];
        for (int j = 0; j < 9; j++){
            col[j] = colours[i * 9 + j];
        }
        int col_int = (int)strtol(col, NULL, 0);
        colour[i][RED] = (0xff & col_int);
        colour[i][GREEN] = (0xff00 & col_int) >> 8;
        colour[i][BLUE] = (0xff0000 & col_int) >> 16;
    }

    fscanf(config, "*\n%i", steps);

    return colour;
}


uint8_t **smoothing(uint8_t **colours_step, int steps, int len){
    /*
    we're linearly interpolating
    */

    uint8_t **all_colours = calloc(steps * len, sizeof(uint8_t*));
    for (int i = 0; i < steps * len; i++){
        all_colours[i] = calloc(3, sizeof(uint8_t));
        int step_n =(int) floor(i / steps);
        double t = (double) (i % steps) / (double) steps;
        all_colours[i][RED] = (uint8_t) ((double) (colours_step[(step_n + 1) % len][RED]) * t) + colours_step[step_n][RED];
        all_colours[i][GREEN] = (uint8_t) ((double) (colours_step[(step_n + 1) % len][GREEN]) * t) + colours_step[step_n][GREEN];
        all_colours[i][BLUE] = (uint8_t) ((double) (colours_step[(step_n + 1) % len][BLUE]) * t) + colours_step[step_n][BLUE];

        //printf("%i %i %i %i \n", i,all_colours[i][RED], all_colours[i][GREEN], all_colours[i][BLUE]);

    }

   return all_colours;
}



int main(int argc, char **argv){

    int t = time(NULL);

    if (argc != 3){
        printf("improper use of this program, use it as such : ./mandelbrot <config file> <output file>\n");

        return EXIT_FAILURE;
    }

    FILE *config = fopen(argv[1], "r");

    FILE *output = fopen(argv[2], "w+");

    int x_size;
    int y_size;
    double x_center;
    double y_center;
    double inter_len;
    int nb; // for part 3
    int iter_max;

    fscanf(config, "%i\n%i\n%lf\n%lf\n%lf\n%i\n%i", &x_size, &y_size, &x_center, &y_center, &inter_len, &nb, &iter_max);

    int col_step;
    int nb_col;
    uint8_t **colours = get_colours(config, &col_step, &nb_col);
    uint8_t **all_colours = smoothing(colours, col_step, nb_col);

    double x_min = x_center - inter_len / 2.0;
    double y_min = y_center - inter_len / 2.0;
    double x_max = x_center + inter_len / 2.0;
    double y_max = y_center + inter_len / 2.0;

    fprintf(output, "P6\n%i  %i\n255\n", x_size, y_size);

    uint32_t image;
    complex pixels;
    int div_time;

    for (int i = 0 ; i < x_size * y_size; i++){
        pixels = translates_int_to_complex(i, x_min, x_max, y_min, y_max, x_size, y_size);
        div_time = diverge(pixels, iter_max);
        image = time_to_colour(div_time, all_colours, nb_col * col_step);
        fwrite((const void *) &image, 3, 1, output);

        int current_time = time(NULL) - t;
        double fraction_done =  (double) i / (double) (x_size * y_size);
        int time_left = (int) ((double) current_time / fraction_done * (1.0 - fraction_done));

        fprintf(stdout, "\rwe're %lf percent done in %li seconds, estimated time until the end : %i seconds                    ", fraction_done * 100, time(NULL) - t, time_left);
        fflush(stdout);
    }

    fclose(output);

    t = time(NULL) - t;

    printf("\rrendering complete %s in %i seconds\n", argv[2], t);
    return EXIT_SUCCESS;
}