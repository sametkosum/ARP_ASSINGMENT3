#include <bmpfile.h>
#include <math.h>

// Map configuration
#define WIDTH 1600
#define HEIGHT 600
#define DEPTH 4
#define RADIUS 30
#define BMP_PATH "out/screenshot.bmp"


struct msg                                                // Design a data structure to hold the location of the circle on the 1600x600 layout

{
    int buf[WIDTH][HEIGHT];
};

void POSITION_OBTAIN(void* data_pointer  , int* position)  // Funtion to get the circle's position
{
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            if (((struct msg*)data_pointer  )->buf[i][j] == 1)
            {
                position[0] = i + RADIUS;
                position[1] = j;
                return;
            }
        }
    }
}

void CIRCLE_DRAWbmp(bmpfile_t *bmp, int x, int y)         // Function to render a blue circular shape in the bmp format

{
    rgb_pixel_t pixel = { 255, 0, 0, 0 };

    for (int i = -RADIUS; i <= RADIUS; i++)
    {
        for (int j = -RADIUS; j <= RADIUS; j++)
        {
            if(sqrt(i*i + j*j) < RADIUS)
                bmp_set_pixel(bmp, x*20 + i, y*20 + j, pixel);
        }
  }
}



void REMOVE_DRAWbmp(bmpfile_t *bmp, int x, int y)            // Funntion to reset the bmp to a white state
{
    rgb_pixel_t pixel = { 255, 255, 255, 0 };

    for (int i = -RADIUS; i <= RADIUS; i++)
    {
        for (int j = -RADIUS; j <= RADIUS; j++)
        {
            if(sqrt(i*i + j*j) < RADIUS)
                bmp_set_pixel(bmp, x*20 + i, y*20 + j, pixel);
        }
    }
}