/*
#include "address_map_nios2.h"
*/

/* include library */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>


/* Screen size. */
#define SCREEN_X 320
#define SCREEN_Y 240
#define BLANK 0x0000
#define BOARD_COLOR 0xFFFF

#define RED     0xF800  // Red 
#define GREEN   0x07E0  // Green
#define BLUE    0x001F  // Blue
#define YELLOW  0xFFE0  // Yellow
#define CYAN    0x07FF  // Cyan
#define MAGENTA 0xF81F  // Magenta
#define ORANGE  0xFD20  // Orange
#define WHITE   0xFFFF // white

#define board_dim_x 130
#define board_dim_y 180
	
#define BLOCK_SIZE 10

int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

typedef struct {
    short int shape[4][4];
} TetrisBlock;

TetrisBlock blocks[7] = {
    // I-Block
    {
        {{WHITE, WHITE, WHITE, WHITE},
         {CYAN, CYAN, CYAN, CYAN},
         {WHITE, WHITE, WHITE, WHITE},
         {WHITE, WHITE, WHITE, WHITE}}
    },
    // J-Block
    {
        {{BLUE, WHITE, WHITE, WHITE},
         {BLUE, BLUE, BLUE, WHITE},
         {WHITE, WHITE, WHITE, WHITE},
         {WHITE, WHITE, WHITE, WHITE}}
    },
    // L-Block
    {
        {{WHITE, WHITE, ORANGE, WHITE},
         {ORANGE, ORANGE, ORANGE, WHITE},
         {WHITE, WHITE, WHITE, WHITE},
         {WHITE, WHITE, WHITE, WHITE}}
    },
    // O-Block
    {
        {{YELLOW, YELLOW, WHITE, WHITE},
         {YELLOW, YELLOW, WHITE, WHITE},
         {WHITE, WHITE, WHITE, WHITE},
         {WHITE, WHITE, WHITE, WHITE}}
    },
    // S-Block
    {
        {{WHITE, GREEN, GREEN, WHITE},
         {GREEN, GREEN, WHITE, WHITE},
         {WHITE, WHITE, WHITE, WHITE},
         {WHITE, WHITE, WHITE, WHITE}}
    },
    // T-Block
    {
        {{WHITE, MAGENTA, WHITE, WHITE},
         {MAGENTA, MAGENTA, MAGENTA, WHITE},
         {WHITE, WHITE, WHITE, WHITE},
         {WHITE, WHITE, WHITE, WHITE}}
    },
    // Z-Block
    {
        {{RED, RED, WHITE, WHITE},
         {WHITE, RED, RED, WHITE},
         {WHITE, WHITE, WHITE, WHITE},
         {WHITE, WHITE, WHITE, WHITE}}
    }
};

short int board[board_dim_x][board_dim_y];
short int random_block[4][4];
void initialize_data();
void draw_tetris_board();
void plot_pixel(int x, int y, short int line_colour);
void wait_for_vsync();
void clear_screen_init();
void draw_box(int x, int y, short int colour);
void swap(int *a, int *b);
void update_board();
void generate_random_block();

int main(void){
    // Set for randomness each time
    srand(time(NULL));

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
    initialize_data();
    bool game_state = true;
    while(game_state){
		generate_random_block();
		update_board();
		clear_screen_init();
        draw_tetris_board();
        // Swaps the front buffer with the back onces the sync is complete and rendering is done
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }
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


// Draws boxes of 1x1 size
void draw_box(int x, int y, short int colour){  //four pixels in total
    for (int i = 0; i < 1; i++){
        for (int j = 0; j < 1; j++){
            plot_pixel(x + i, y + j, colour);
        }
    }
}


// Function draws the initial white board where the block will drop in
void draw_tetris_board() {
    for (int i = 0; i < board_dim_x; i++) {
        for (int j = 0; j < board_dim_y; j++) {
            // Adjusting coordinates based on i and j
            int x = 70 + i;
            int y = 40 + j;
            short int color = board[i][j];
            plot_pixel(x, y, color);
        }
    }
}


// Function initializes any data that requires to be set before game starts
void initialize_data(){
    for (int i = 0; i < board_dim_x; i++) {
        for (int j = 0; j < board_dim_y; j++) {
            board[i][j] = BOARD_COLOR;
        }
    }
}

// Generate a random block from the blocks list and place it in the global var for later use.
void generate_random_block(){
    // Generate random number
    int random_num = rand() % 7;
    // Copy the selected block from the blocks array
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            random_block[i][j] = blocks[random_num].shape[i][j];
        }
    }
}

// Function updates the board by putting the random block at the top of the board
void update_board() {
    // Define the starting position for placing the block
    int start_x = board_dim_x / 2 - (BLOCK_SIZE/2); // Adjusted to center the block
    int start_y = 0;

    // Loop through each cell of the BLOCK_SIZE x BLOCK_SIZE block
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            // Calculate the corresponding position on the board
            int x = start_x + i * BLOCK_SIZE;
            int y = start_y + j * BLOCK_SIZE;

            // Update the board with the color of the block
            if (x >= 0 && x < board_dim_x && y >= 0 && y < board_dim_y) {
                // Update the board with the block color
                for (int block_x = 0; block_x < BLOCK_SIZE; block_x++) {
                    for (int block_y = 0; block_y < BLOCK_SIZE; block_y++) {
                        board[x + block_x][y + block_y] = random_block[i][j];
                    }
                }
            }
        }
    }
}