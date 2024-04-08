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

#define WIDTH 10
#define HEIGHT 20

#define board_x_pixels 90
#define board_y_pixels 180
	
#define BLOCK_SIZE 9
#define SHAPE_ARRAY_SIZE 4

int pixel_buffer_start; // global variable
short int Buffer1[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

typedef struct {
    short int shape[4][4];
    int x;
    int y;
    int color;
} TetrisBlock;

TetrisBlock blocks[7] = {
    // I-Block
    {
        {{BLANK, BLANK, BLANK, BLANK},
         {CYAN, CYAN, CYAN, CYAN},
         {BLANK, BLANK, BLANK, BLANK},
         {BLANK, BLANK, BLANK, BLANK}}
    },
    // J-Block
    {
        {{BLUE, BLANK, BLANK, BLANK},
         {BLUE, BLUE, BLUE, BLANK},
         {BLANK, BLANK, BLANK, BLANK},
         {BLANK, BLANK, BLANK, BLANK}}
    },
    // L-Block
    {
        {{BLANK, BLANK, ORANGE, BLANK},
         {ORANGE, ORANGE, ORANGE, BLANK},
         {BLANK, BLANK, BLANK, BLANK},
         {BLANK, BLANK, BLANK, BLANK}}
    },
    // O-Block
    {
        {{YELLOW, YELLOW, BLANK, BLANK},
         {YELLOW, YELLOW, BLANK, BLANK},
         {BLANK, BLANK, BLANK, BLANK},
         {BLANK, BLANK, BLANK, BLANK}}
    },
    // S-Block
    {
        {{BLANK, GREEN, GREEN, BLANK},
         {GREEN, GREEN, BLANK, BLANK},
         {BLANK, BLANK, BLANK, BLANK},
         {BLANK, BLANK, BLANK, BLANK}}
    },
    // T-Block
    {
        {{BLANK, MAGENTA, BLANK, BLANK},
         {MAGENTA, MAGENTA, MAGENTA, BLANK},
         {BLANK, BLANK, BLANK, BLANK},
         {BLANK, BLANK, BLANK, BLANK}}
    },
    // Z-Block
    {
        {{RED, RED, BLANK, BLANK},
         {BLANK, RED, RED, BLANK},
         {BLANK, BLANK, BLANK, BLANK},
         {BLANK, BLANK, BLANK, BLANK}}
    }
};

short int board[WIDTH][HEIGHT];
TetrisBlock random_block;
TetrisBlock previous_block;
bool game_state = true;

void plot_pixel(int x, int y, short int line_colour);
void wait_for_vsync();
void clear_screen_init();
void draw_box(int x, int y, short int colour);
void swap(int *a, int *b);


void initialize_data();
void draw_tetris_board();
bool update_board();
void generate_random_block();
void clear_prev_block();
void clear_prev_block();
void merge_block();
void update_block_location();
bool is_in_bounds();
bool does_overlap();
bool new_block = true;
bool down = false;
bool left = false;
bool right = false;


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
    
	
    while(game_state){
		if(new_block){
			generate_random_block();
		}
        clear_prev_block();
        merge_block();
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

// Draws boxes of 4x4 size
void draw_box(int x, int y, short int colour){  //four pixels in total
    for (int i = 0; i < 1; i++){
        for (int j = 0; j < 1; j++){
            plot_pixel(x + i, y + j, colour);
        }
    }
}

// Function draws the initial white board where the block will drop in
void draw_tetris_board() {
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            // Adjusting coordinates based on i and j
            int x = 40 + i * BLOCK_SIZE; // Multiply by 10 for scaling
            int y = 40 + j * BLOCK_SIZE; // Multiply by 10 for scaling
            short int color = board[i][j];
            
            // Draw a 10x10 square for each board element
            for (int k = 0; k < BLOCK_SIZE; k++) {
                for (int l = 0; l < BLOCK_SIZE; l++) {
                    draw_box(x + k, y + l, color);
                }
            }
        }
    }
}

// Function initializes any data that requires to be set before game starts
void initialize_data(){
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            board[i][j] = BOARD_COLOR;
        }
    }
    // Initialize the previous block struct
    for (int i = 0; i < SHAPE_ARRAY_SIZE; i++){
        for (int j = 0; j < SHAPE_ARRAY_SIZE; j++){
            previous_block.shape[i][j] = WHITE;
        }
    }
}

// Generate a random block from the blocks list and place it in the global var for later use.
void generate_random_block(){
    int colors[7] = {CYAN, BLUE, ORANGE, YELLOW, GREEN, MAGENTA, RED};
    // Generate random number
    int random_num = rand() % 7;
	printf("Random number - %d\n", random_num);
    // Copy the selected block from the blocks array
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            random_block.shape[i][j] = blocks[random_num].shape[i][j];
        }
    }
	// always initialize its coordinates towards the top-center of the board
    random_block.x = WIDTH / 2 - 1;
    random_block.y = 0;
    random_block.color = colors[random_num];
	
	previous_block.y = 0;
	previous_block.x = WIDTH / 2 - 1;
    previous_block.color = random_block.color;
	new_block = false;
}

void merge_block(){
	update_block_location();
	if (!is_in_bounds()){
        random_block.x = previous_block.x;
        random_block.y = previous_block.y;
        new_block = true;
	}
	
    for (int i = 0; i < SHAPE_ARRAY_SIZE; i++){
		for (int j = 0; j < SHAPE_ARRAY_SIZE; j++){
			if ((random_block.shape[i][j] != BLANK)){
				board[random_block.x + i][random_block.y + j] = random_block.shape[i][j];
			}
		}
	}
}


void update_block_location(){
    previous_block.x = random_block.x;
    previous_block.y = random_block.y;

    //right = true;
    
    if(down){
        random_block.y += 1;
    }
    if (right){
		if (random_block.x > WIDTH - 1){
            random_block.x = WIDTH - 2;
        }else{
            random_block.x += 1;
        }
    }
    if (left){
        if(random_block.x < 0){
            random_block.x = 0;
        } else {
            random_block.x -= 1;
        }
    }
	random_block.y += 1;
}

void clear_prev_block(){
	int board_x = previous_block.x;
	int board_y = previous_block.y;
	for (int i = 0; i < SHAPE_ARRAY_SIZE; i++){
		for (int j = 0; j < SHAPE_ARRAY_SIZE; j++){
			board[board_x + i][board_y + j] = previous_block.shape[i][j];
		}
	}
}

bool is_in_bounds(){
    for (int i = 0; i < SHAPE_ARRAY_SIZE; i++){
		for (int j = 0; j < SHAPE_ARRAY_SIZE; j++){
            if ((random_block.shape[i][j] != BLANK)){
			    if(random_block.x + i >= WIDTH || random_block.y + j >= HEIGHT || random_block.x + i < 0){
                    return false;
                }
            }
		}
	}
    return true;
}

bool does_overlap(){
	for (int i = 0; i < SHAPE_ARRAY_SIZE; i++){
		for (int j = 0; j < SHAPE_ARRAY_SIZE; j++){
			if ((random_block.shape[i][j] != BLANK)){
				if (board[random_block.x + i][random_block.y + j] != WHITE){
                    return true;
                }
			}
		}
	}
	return false;
}