
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "display_cst.h"

#define RGB_RED 0
#define RGB_GREEN 1
#define RGB_BLUE 2

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
    int colour = colours[time][RGB_RED] + (colours[time][RGB_GREEN] << 8) + (colours[time][RGB_BLUE] << 16);
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
        colour[i][RGB_RED] = (0xff & col_int);
        colour[i][RGB_GREEN] = (0xff00 & col_int) >> 8;
        colour[i][RGB_BLUE] = (0xff0000 & col_int) >> 16;
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
        all_colours[i][RGB_RED] = (uint8_t) ((double) (colours_step[(step_n + 1) % len][RGB_RED]) * t) + colours_step[step_n][RGB_RED];
        all_colours[i][RGB_GREEN] = (uint8_t) ((double) (colours_step[(step_n + 1) % len][RGB_GREEN]) * t) + colours_step[step_n][RGB_GREEN];
        all_colours[i][RGB_BLUE] = (uint8_t) ((double) (colours_step[(step_n + 1) % len][RGB_BLUE]) * t) + colours_step[step_n][RGB_BLUE];

        //printf("%i %i %i %i \n", i,all_colours[i][RED], all_colours[i][GREEN], all_colours[i][BLUE]);

    }

   return all_colours;
}


void print_colour(int red, int green, int blue){
    int closest_colour = 0;
    int closest_distance = INT32_MAX;

    int colours[64][3] = {
        {23,20,33},    //BLACK_FULL
        {29,18,33},    //BLACK_3QUART
        {35,15,34},    //BLACK_HALF
        {41,12,35},    //BLACK_QUART
        {192,28,40},   //RED_FULL
        {156,24,39},   //RED_3QUART
        {120,19,38},   //RED_HALF
        {84,14,37},    //RED_QUART
        {38,162,105},  //GREEN_FULL
        {40,124,88},   //GREEN_3QUART
        {43,86,70},    //GREEN_HALF
        {45,47,53},    //GREEN_QUART
        {162,115,76},  //YELLOW_FULL
        {133,89,66},   //YELLOW_3QUART
        {105,62,56},   //YELLOW_HALF
        {76,35,46},    //YELLOW_QUART
        {76,35,46},    //BLUE_FULL
        {25,57,113},   //BLUE_3QUART
        {33,41,87},    //BLUE_HALF
        {40,25,61},    //BLUE_QUART
        {163,71,186},  //MAGENTA_FULL
        {134,56,149},  //MAGENTA_3QUART
        {105,40,111},  //MAGENTA_HALF
        {76,24,73},    //MAGENTA_QUART
        {42,161,179},  //CYAN_FULL
        {43,124,143},  //CYAN_3QUART
        {45,85,107},   //CYAN_HALF
        {46,47,71},    //CYAN_HALF
        {208,207,204}, //WHITE_FULL
        {168,158,162}, //WHITE_3QUART
        {128,108,120}, //WHITE_HALF
        {88,58,78},    //WHITE_QUART
        {94,92,100},   //BRIGHT_BLACK_FULL
        {82,72,84},    //BRIGHT_BLACK_3QUART
        {71,51,68},    //BRIGHT_BLACK_HALF
        {59,30,52},    //BRIGHT_BLACK_QUART
        {246,97,81},   //BRIGHT_RED_FULL
        {197,76,69},   //BRIGHT_RED_3QUART
        {147,53,58},   //BRIGHT_RED_HALF
        {97,31,47},    //BRIGHT_RED_QUART
        {51,218,122},  //BRIGHT_GREEN_FULL
        {50,167,100},  //BRIGHT_GREEN_3QUART
        {49,114,79},   //BRIGHT_GREEN_HALF
        {48,61,57},    //BRIGHT_GREEN_QUART
        {233,173,12},  //BRIGHT_YELLOW_FULL
        {187,133,18},  //BRIGHT_YELLOW_3QUART
        {140,91,24},   //BRIGHT_YELLOW_HALF
        {94,50,30},    //BRIGHT_YELLOW_QUART
        {42,123,222},  //BRIGHT_BLUE_FULL
        {43,95,176},   //BRIGHT_BLUE_3QUART
        {45,66,129},   //BRIGHT_BLUE_HALF
        {46,37,82},    //BRIGHT_BLUE_QUART
        {192,97,203},  //BRIGHT_MAGENTA_FULL
        {56,76,161},   //BRIGHT_MAGENTA_3QUART
        {120,53,119},  //BRIGHT_MAGENTA_HALF
        {84,31,77},    //BRIGHT_MAGENTA_QUART
        {51,199,222},  //BRIGHT_CYAN_FULL
        {50,152,176},  //BRIGHT_CYAN_3QUART
        {49,104,129},  //BRIGHT_CYAN_HALF
        {48,56,82},    //BRIGHT_CYAN_HALF
        {255,255,255}, //BRIGHT_WHITE_FULL
        {203,194,200}, //BRIGHT_WHITE_3QUART
        {152,133,146}, //BRIGHT_WHITE_HALF
        {100,71,91}    //BRIGHT_WHITE_QUART
    };


    for (int i = 0; i < 64; i++){
        int red_real = red;
        int green_real = green;
        int blue_real = blue;
        int red_text = colours[i][RGB_RED];
        int green_text = colours[i][RGB_GREEN];
        int blue_text = colours[i][RGB_BLUE];
        int distance = (red_real - red_text) * (red_real - red_text) + 
                       (green_real - green_text) * (green_real - green_text) + 
                       (blue_real - blue_text) * (blue_real - blue_text);
        if (distance < closest_distance){
            closest_colour = i;
            closest_distance = distance;
        }
    }

    switch (closest_colour / 4) {
        case 0:
            printf("%s", BLACK);
            break;
        case 1:
            printf("%s", RED);
            break;
        case 2:
            printf("%s", GREEN);
            break;
        case 3:
            printf("%s", YELLOW);
            break;
        case 4:
            printf("%s", BLUE);
            break;
        case 5:
            printf("%s", MAGENTA);
            break;
        case 6:
            printf("%s", CYAN);
            break;
        case 7:
            printf("%s", WHITE);
            break;
        case 8:
            printf("%s", BRIGHT_BLACK);
            break;
        case 9:
            printf("%s", BRIGHT_RED);
            break;
        case 10:
            printf("%s", BRIGHT_GREEN);
            break;
        case 11:
            printf("%s", BRIGHT_YELLOW);
            break;
        case 12:
            printf("%s", BRIGHT_BLUE);
            break;
        case 13:
            printf("%s", BRIGHT_MAGENTA);
            break;
        case 14:
            printf("%s", BRIGHT_CYAN);
            break;
        case 15:
            printf("%s", BRIGHT_WHITE);
            break;
    }

    switch (closest_colour % 4) {
        case 0:
            printf("%s", FULL_BLOCK);
            break;
        case 1:
            printf("%s", THREE_QUART_BLOCK);
            break;
        case 2:
            printf("%s", HALF_BLOCK);
            break;
        case 3:
            printf("%s", QUART_BLOCK);
            break;
    }
}

void display_preview(double x_min, double x_max, double y_min, double y_max, int iter_max, uint8_t **all_colours, int nb_col, int col_step){

    uint32_t image;
    complex pixels;
    int div_time;

    int x_size = 40;
    int y_size = 40;

    printf("preview : \n");

    for (int i = 0 ; i < x_size * y_size; i++){
        pixels = translates_int_to_complex(i, x_min, x_max, y_min, y_max, x_size, y_size);
        div_time = diverge(pixels, iter_max);
        image = time_to_colour(div_time, all_colours, nb_col * col_step);

        int red = image &   0x000000ff;
        int green = (image & 0x0000ff00) >> 8;
        int blue = (image &  0x00ff0000) >> 16;

        if (i % x_size == 0){
            printf("\n");
        }

        print_colour(red, green, blue);

        fflush(stdout);
    }
    printf("%s\n",BRIGHT_WHITE);
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

    display_preview(x_min, x_max, y_min, y_max, iter_max, all_colours, nb_col, col_step);

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

    printf("\rrendering of %s complete in %i seconds                                                                       \n", argv[2], t);
    return EXIT_SUCCESS;
}