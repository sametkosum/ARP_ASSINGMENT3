#include "./../include/processA_utilities.h"
#include "./../include/voids.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Semaphore configuration
#define SHARED_MEM_NAME "/my_shared_mem"
#define WRITE_SEM_NAME "/my_write_sem"
#define READ_SEM_NAME "/my_read_sem"
#define SHM_MODE 0666
#define SHM_NAME "/AOS"
#define SEM_MODE 0644


int main(int argc, char *argv[])
{
    int numberOfport = atoi(argv[1]);
    struct sockaddr_in IP_ADRESS;                      // Assign socket to address and port number
    IP_ADRESS.sin_family = AF_INET;
    IP_ADRESS.sin_port = htons(numberOfport);
    IP_ADRESS.sin_addr.s_addr = INADDR_ANY;
 
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);   // Produce a socket
    if (server_fd == -1)
    {
        perror("Unable to create socket\n");
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr *)&IP_ADRESS, sizeof(IP_ADRESS)) == -1)
    {    
        perror("Cannot bind socket\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
 
    if (listen(server_fd, 1) == -1)              // Listen for incoming connections
    {
        perror("Cannot listen on socket\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    int addr_len = sizeof(IP_ADRESS);                  // Accept incoming connection
    int client_sock = accept(server_fd, (struct sockaddr *)&IP_ADRESS, (socklen_t *)&addr_len);
    if (client_sock == -1)
    {
        perror("Cannot accept connection\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    int shared_mem_descriptor  = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, SHM_MODE); // Produce a shared memory section
    if (shared_mem_descriptor  == -1)
    {
        perror("Unable to open shared memory section\n");
        exit(EXIT_FAILURE);
    }
    ftruncate(shared_mem_descriptor , sizeof(struct msg)); // Set the dimensions of the shared memory section

    // Link the shared memory allocation to the process' virtual address space
    void *data_pointer   = mmap(0, sizeof(struct msg), PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem_descriptor , 0);
    if (data_pointer   == MAP_FAILED)
    {
        perror("Shared memory encounter error\n");
        exit(EXIT_FAILURE);
    }
    bmpfile_t *bmp;            // Produce a bmp file of 1600x600x4          
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Initiate semaphore synchronization
    sem_t *write_semaphore  = sem_open(WRITE_SEM_NAME, O_CREAT, SEM_MODE, 1);
    sem_t *read_semaphore  = sem_open(READ_SEM_NAME, O_CREAT, SEM_MODE, 0);

    if (write_semaphore  == SEM_FAILED || read_semaphore  == SEM_FAILED)
    {
        perror("Failed to access the semaphore\n");
        exit(EXIT_FAILURE);
    }
    sem_wait(write_semaphore );                  // Draw the circle in its initial position
    CIRCLE_DRAWbmp(bmp, circle.x, circle.y);    // Draw circle
     
    for (int i = 0; i < WIDTH; i++)              //Store the circular information within the shared memory
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
        int previous_x  = circle.x, previous_y = circle.y;
        int cmd = getch();                     // Get input in non-blocking mode
        if(cmd == KEY_RESIZE)                  // If user resizes screen, re-draw UI
        {
            if(first_resize)
                first_resize = FALSE;
            else
                reset_console_ui();
        }
        else if(cmd == KEY_MOUSE)               // Else if user presses print button
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
        else if(read(client_sock, &cmd, sizeof(cmd)) > 0)       // Else if input is an arrow key, move circle accordingly
        {
            sem_wait(write_semaphore );
            move_circle(cmd);                                   // Move circle
            draw_circle();
            REMOVE_DRAWbmp(bmp, previous_x , previous_y);       // Remove previous circle
           
            for (int i = 0; i < WIDTH; i++)                     // Reset shared memory data pointer as well
            {
                for (int j = 0; j < HEIGHT; j++)
                    ((struct msg*)data_pointer  )->buf[i][j] = 0;
            }

            CIRCLE_DRAWbmp(bmp, circle.x, circle.y);            // Draw new circle

            for (int i = 0; i < WIDTH; i++)                      //Store the circular information within the shared memory_pointer
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
    bmp_destroy(bmp);                              // Deallocate bmp memory
    munmap(data_pointer  , sizeof(struct msg));    // Unmap shared memory allocation
    close(shared_mem_descriptor );                 // Close shared memory allocation
    sem_close(write_semaphore );                   // Close semaphores
    sem_close(read_semaphore );
    close(client_sock);                            // Close client socket
    shutdown(server_fd, SHUT_RDWR);                // Close server socket
    shm_unlink(SHARED_MEM_NAME);                   // Unlink shared memory allocation
    sem_unlink(WRITE_SEM_NAME);
    sem_unlink(READ_SEM_NAME);
    endwin();                                      // Close UI
    return 0;
}
