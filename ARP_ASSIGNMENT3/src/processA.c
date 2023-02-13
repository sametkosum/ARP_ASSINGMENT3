#include "./../include/processA_utilities.h"
#include "./../include/voids.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <bmpfile.h>
#include <math.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <bmpfile.h>

// Semaphore configuration

#define SHARED_MEM_NAME "/my_shared_mem"
#define WRITE_SEM_NAME "/my_write_sem"
#define READ_SEM_NAME "/my_read_sem"
#define SHM_MODE 0666
#define SHM_NAME "/AOS"
#define SEM_MODE 0644

int main(int argc, char *argv[])
{
    int shared_mem_descriptor  = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, SHM_MODE);   // Produce a shared memory section
    if (shared_mem_descriptor  == -1)
    {
        perror("Unable to open shared memory section\n");
        return 1;
    }
    ftruncate(shared_mem_descriptor , sizeof(struct msg));     // Set the dimensions of the shared memory section
    void *data_pointer   = mmap(0, sizeof(struct msg), PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_descriptor , 0); // Link the shared memory allocation to the process' virtual address space
    if (data_pointer   == MAP_FAILED)
    {
        perror("Shared memory encounter error\n");
        return 1;
    }
    bmpfile_t *bmp;                                                             // Produce a bmp file of 1600x600x4
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);
    int first_resize = TRUE;                                                    // Utility variable to avoid trigger resize event on launch
    init_console_ui();                                                          // Initialize UI
    sem_t *write_semaphore  = sem_open(WRITE_SEM_NAME, O_CREAT, SEM_MODE, 1);   // Initiate semaphore synchronization
    sem_t *read_semaphore  = sem_open(READ_SEM_NAME, O_CREAT, SEM_MODE, 0);     // Initiate semaphore synchronization
    if (write_semaphore  == SEM_FAILED || read_semaphore  == SEM_FAILED)
    {
        perror("Failed to access the semaphore\n");
        return 1;
    }
    sem_wait(write_semaphore );                // Draw the circle in its initial position
    CIRCLE_DRAWbmp(bmp, circle.x, circle.y);  // Draw circle
    for (int i = 0; i < WIDTH; i++)            //Store the circular information within the shared memory_pointer  
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            rgb_pixel_t *pixel = bmp_get_pixel(bmp, i, j);
            if ((pixel->blue == 255) && (pixel->red == 0) && (pixel->green == 0) && (pixel->alpha == 0))
                ((struct msg*)data_pointer  )->buf[i][j] = 1;
        }
    }
    sem_post(read_semaphore );
    while (TRUE)
    {
        int previous_x  = circle.x, previous_y = circle.y;    // Get input in non-blocking mode
        int cmd = getch();
        if(cmd == KEY_RESIZE)                                 // If user resizes screen, re-draw UI
        {
            if(first_resize)
                first_resize = FALSE;
            else
                reset_console_ui();
        }
        // Else if user presses print button
        else if(cmd == KEY_MOUSE)
        {
            if(getmouse(&event) == OK)
            {
                if(check_button_pressed(print_btn, &event))
                {
                    mvprintw(LINES - 1, 1, "Snapshot recorded");
                    bmp_save(bmp, BMP_PATH);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++)
                        mvaddch(LINES - 1, j, ' ');
                }
            }
        }
        // Else if input is an arrow key, move circle accordingly
        else if(cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN)
        {
            sem_wait(write_semaphore );   
            move_circle(cmd);
            draw_circle(); 
            REMOVE_DRAWbmp(bmp, previous_x , previous_y);
            for (int i = 0; i < WIDTH; i++)// Reset shared memory data pointer as well
            {
                for (int j = 0; j < HEIGHT; j++)
                    ((struct msg*)data_pointer  )->buf[i][j] = 0;
            }
            CIRCLE_DRAWbmp(bmp, circle.x, circle.y);                 // Draw new circle
            for (int i = 0; i < WIDTH; i++)                           //Store the circular information within the shared memory_pointer
            {
                for (int j = 0; j < HEIGHT; j++)
                {
                    rgb_pixel_t *pixel = bmp_get_pixel(bmp, i, j);
                    if ((pixel->blue == 255) && (pixel->red == 0) && (pixel->green == 0) && (pixel->alpha == 0))
                        ((struct msg*)data_pointer  )->buf[i][j] = 1;
                }
            }
            sem_post(read_semaphore );
        }
    }
    bmp_destroy(bmp);
    munmap(data_pointer  , sizeof(struct msg));
    close(shared_mem_descriptor );
    sem_close(write_semaphore );
    sem_close(read_semaphore );
    shm_unlink(SHARED_MEM_NAME);
    sem_unlink(WRITE_SEM_NAME);
    sem_unlink(READ_SEM_NAME);
    endwin();
    return 0;
}
