#include <stdlib.h>
#include <stdio.h>

#define MAX 1000000

int main(){

    for (int i = 0; i < MAX ; i++){
        fprintf(stdout, "\rwe're at %lf", (double) i/ (double) MAX * 100);
        fflush(stdout);
    }
    fprintf(stdout, "\n");
    fflush(stdout);
    
    return EXIT_SUCCESS;
}