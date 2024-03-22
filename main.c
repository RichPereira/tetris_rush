/*
#include "address_map_nios2.h"
*/

/* include library */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


/* Screen size. */
#define SCREEN_X 320
#define SCREEN_Y 240
#define BLANK 0x0000
#define N 8

#define board_dim_x 100
#define board_dim_y 200

int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];
void initialize_board();
void plot_pixel(int x, int y, short int line_colour);
void wait_for_vsync();
void clear_screen_init();
void draw_box(int x, int y, short int colour);
void swap(int *a, int *b);

int main(void){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    /* set front pixel buffer to Buffer 1 */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer1; // first store the address in the  back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen_init(); // pixel_buffer_start points to the pixel buffer

    /* set back pixel buffer to Buffer 2 */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen_init(); // pixel_buffer_start points to the pixel buffer

    
    return 0;
}


// Draws the pixel given an x and y coordinate and a colour
void plot_pixel(int x, int y, short int line_colour){
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_colour;
}

// Function waits for the S bit to become 0 again - means synchronization is complete
void wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020; // pixel (DMA) controller (I/O)
    int status;
    *pixel_ctrl_ptr = 1; // start synchronization; s bit is set to 1
    status = *(pixel_ctrl_ptr + 3); // read status register at address
    while ((status & 0x01) != 0){
        status = *(pixel_ctrl_ptr+3);
    }
}

// clears the whole screen given the the screen coordinates and drawing blank pixels
void clear_screen_init(){
    for(int temp_x = 0; temp_x < SCREEN_X; temp_x++){
        for(int temp_y = 0; temp_y < SCREEN_Y; temp_y++){
            plot_pixel(temp_x, temp_y, BLANK);  //draw  0x0000 everywhere
        }
    }
}

// Function swaps two values with each other using pointers
void swap(int *a, int *b){
    int temp = *a;
    *a = *b;
    *b = temp;
}


void draw_box(int x, int y, short int colour){  //four pixels in total
    plot_pixel(x, y, colour);
    plot_pixel(x + 1, y, colour);
    plot_pixel(x, y + 1, colour);
    plot_pixel(x + 1, y + 1, colour);
}