#include "./../include/processB_utilities.h"
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


int main(int argc, char const *argv[])
{
    int shared_mem_descriptor  = shm_open(SHARED_MEM_NAME, O_RDONLY, SHM_MODE); // Produce a shared memory section
    if (shared_mem_descriptor  == -1)
    {
        perror("Unable to open shared memory section\n");
        return 1;
    }
    void *data_pointer   = mmap(0, sizeof(struct msg), PROT_READ, MAP_SHARED, shared_mem_descriptor , 0); // Link the shared memory allocation to the process' virtual address space
    if (data_pointer   == MAP_FAILED)
    {
        perror("Shared memory encounter error\n");
        return 1;
    }
    bmpfile_t *bmp;                                           // Produce a bmp file of 1600x600x4
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);
    int first_resize = TRUE;                                  // Utility variable to avoid trigger resize event on launch
    init_console_ui();                                        // Initialize UI
    sem_t *write_semaphore  = sem_open(WRITE_SEM_NAME, 0);    // Open semaphore
    sem_t *read_semaphore  = sem_open(READ_SEM_NAME, 0);      // Open semaphore
    if (write_semaphore  == SEM_FAILED || read_semaphore  == SEM_FAILED)
    {
        perror("ERROR to access the semaphore\n");
        return 1;
    }
    int position[2];                                          // index 0 refers to the x-coordinate and index 1 refers to the y-coordinate
    while (TRUE)
    {
        int previous_position[2] = { position[0], position[1] };   
        int cmd = getch();                                    // Get input in non-blocking mode
        if(cmd == KEY_RESIZE)                                 // If user resizes screen, re-draw UI
        {
            if(first_resize)
                first_resize = FALSE;
            else
                reset_console_ui();
        }
        else
        {
            sem_wait(read_semaphore );
            POSITION_OBTAIN(data_pointer  , position);        // Draw a '0' in the position of the circle from process A
            mvaddch((int)round(position[1]/20), (int)round(position[0]/20), '0'); 
            REMOVE_DRAWbmp(bmp, previous_position[0], previous_position[1]); // Create a copy of the image from process A
            CIRCLE_DRAWbmp(bmp, position[0], position[1]);
            refresh();
            sem_post(write_semaphore );
        }
    }

    bmp_destroy(bmp);
    munmap(data_pointer  , sizeof(struct msg));
    close(shared_mem_descriptor );
    sem_close(write_semaphore );
    sem_close(read_semaphore );

    endwin();
    return 0;
}
