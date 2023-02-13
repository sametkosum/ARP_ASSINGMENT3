# ARP assignment 3

Base repository for the **third assignment** of the **_Advanced and Robot Programming_** course - **_UNIGE_**.

Mustafa Melih Toslak 
Ahmet Samet Kosum      S5635830

Project's repository: [assignment 3]()

## The goal of assignment 

The code to design, develop, test and deploy is a modified version of Assignment 2, including client/server features. We refer to this as "application".

![ARP33](https://user-images.githubusercontent.com/117012520/218459680-a434ac0d-a9c9-40c1-b7ee-7a47ca7d2178.PNG)


In the modified application, process B, shared memory and the second ncurses window are
unchanged. Process A includes two new features:
1. client connection toward a similar application running on a different machine in the network
2. server connection for a similar application running on a different machine in the network

Therefore the application, when launched, asks for one execution modality:
1. normal, as assignment 2
2. server: the application does not use the keyboard for input: it receives input from another
application (on a different machine) running in client mode
3. client: the application runs normally as assignment 2 and sends its keyboard input to another
application (on a different machine) running in server mode
When selecting modes 2 and 3 the application obviously asks address and port of the companion
application.

## The program description

This program is a system that involves interprocess communication and display of a moving circle in a terminal window. It consists of five separate C programs: ProcessA.c, ProcessB.c, CLIENT.c, SERVER.c, and Master.c.

ProcessA.c and ProcessB.c communicate with each other using shared memory and semaphores, and display the moving circle in the terminal window using the ncurses library. CLIENT.c and SERVER.c communicate with each other over a network, and the server listens on a specified port number for incoming connections. Master.c forks two child processes, the first of which runs ProcessA in normal, server, or client mode, and the second of which runs ProcessB.

The program greets the user and presents three modes to choose from: normal, server, or client. Based on the mode selected, the arguments for the first child process are set up and the second child process has a fixed set of arguments. After both child processes have completed, the parent process prints an exit message with the exit status of the child processes and returns the exit status of the parent process.

**ProcessA.c** creates a console UI for displaying a moving circle and utilizes shared memory and semaphores for interprocess communication with ProcessB.c. The shared memory stores the position of the circle, and the semaphores ensure that the shared memory is only accessed by one process at a time. The code uses the ncurses library for the console UI and the bmp library for saving a snapshot of the UI as a bitmap image.

**ProcessB.c** displays the moving circle in the terminal window and also communicates with ProcessA.c using System V shared memory and semaphores. It maps the shared memory region to its virtual address space and uses semaphores to ensure mutually exclusive access to the shared memory. The code uses an infinite loop to wait for user input, such as resizing the terminal window or moving the circle. It generates a bitmap image of the moving circle and cleans up the shared memory and semaphores before exiting.

**CLIENT.c** creates creates a client which connects to a server over a network. It performs several operations to establish a connection and transfer information to the server.The program takes two command line arguments, location_server and numberOfport, which are the IP address and port number of the server. It creates a socket using socket() and then connects to the server using connect().The program also creates a shared memory allocation using shm_open(), sets the dimensions of the shared memory using ftruncate(), and maps the shared memory to the process' virtual address space using mmap().A semaphore is also created using sem_open() for synchronization purposes between the client and server. The program then draws a circle and stores the information about the circle in the shared memory.The program enters into an infinite loop and waits for user input. It checks if the user has resized the screen or pressed the print button. If the print button is pressed, the program saves a snapshot of the screen to a BMP file.The program also includes several error checks and closes the socket, shared memory, and semaphore in case of errors. 

**SERVER.c** implements a server which listens on a specified port number for incoming connections. It creates a shared memory section for storing information about a circular shape, as well as semaphores for synchronization purposes. The server also uses ncurses library to handle user inputs and redraw the circular shape on the screen.The server first sets up the socket address structure (IP_ADDRESS) with the given port number and binds the socket to it. It then listens on the socket for incoming connections, and accepts the incoming connection when it arrives.The server then opens a shared memory section using the shm_open function, and sets the size of the shared memory section using the ftruncate function. The shared memory is then linked to the process' virtual address space using the mmap function.The server then creates a bmp image of 1600x600x4 pixels and draws the circular shape in its initial position. The information about the circular shape is stored in the shared memory and the read semaphore is signaled to indicate that the data in the shared memory is ready to be read.The server then enters a loop that waits for user inputs using the ncurses getch function. If the user resizes the screen, the server will redraw the UI using the reset_console_ui function. If the user generates a mouse event, the server will update the position of the circular shape and store the updated information in the shared memory. The write semaphore is used to synchronize access to the shared memory between the server and the client.This code assumes that the user inputs are handled using the ncurses library and the bmp image is stored using the bmp-Images library.

**Master.c** forks two child processes. The first child process is created using the spawn() function, which forks a child process and executes a specified program. The second child process is created in the same way. The first child process runs the "./bin/processA" program, while the second child process runs the "./bin/processB" program. The execution of the parent process is blocked until both child processes complete.

The program first greets the user and presents three modes to choose from: normal, server, or client. The user's choice is stored in the "mode_choice" variable.

Based on the mode selected, the program sets up the arguments for the first child process. If the normal mode is selected, the arguments will be: KONSOLE, "-e", "./bin/processA". If the server mode is selected, the program will ask the user to enter a port number and the arguments will be: KONSOLE, "-e", "./bin/processAserver", port_number. If the client mode is selected, the program will ask the user to enter an IP address and a port number, and the arguments will be: KONSOLE, "-e", "./bin/processAclient", IP_address, port_number.

The second child process has a fixed set of arguments: KONSOLE, "-e", "./bin/processB".

After both child processes have completed, the parent process prints an exit message with the exit status of the child processes and returns the exit status of the parent process.


The project provides a basic implementation of a simulated vision system using _shared memory_, according to the requirements specified in the [PDF file of the assignment](second_assignment.pdf).


In the folder `/src` you will find three processes:

## *libbitmap* installation and usage
To work with the bitmap library, you need to follow these steps:
1. Download the source code from [this GitHub repo](https://github.com/draekko/libbitmap.git) in your file system.
2. Navigate to the root directory of the downloaded repo and run the configuration through command ```./configure```. Configuration might take a while.  While running, it prints some messages telling which features it is checking for.
3. Type ```make``` to compile the package.
4. Run ```make install``` to install the programs and any data files and documentation.
5. Upon completing the installation, check that the files have been properly installed by navigating to ```/usr/local/lib```, where you should find the ```libbmp.so``` shared library ready for use.
6. In order to properly compile programs which use the *libbitmap* library, you first need to notify the **linker** about the location of the shared library. To do that, you can simply add the following line at the end of your ```.bashrc``` file:

```export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"```
### Using *libbitmap* in your code
Now that you have properly installed the library in your system, it's time to use it in your programs:
1. Include the library in your programs via ```#include <bmpfile.h>```. If you want to check the content of ```bmpfile.h``` to glimpse the functionalities of the library, navigate to ```/usr/local/include```, where the header file should be located.
2. Compile programs which use the *libbitmap* library by linking the shared library with the ```-lbmp``` command.     
Example for compiling **circle.c**: ```gcc src/circle.c -lbmp -lm -o bin/circle``` 

## Compiling and running **processA** and **processB**
The two processes are implemented as UIs through *ncurses* library, therefore you need to compile their source files by linking the shared library via ```-lncurses```. As for the first assignment, exploit the resize event of the windows to get out of situations in which the graphical elements do not properly spawn.

## Compiling and running the code

```console
$ ./run.sh
```

## Executing **circle.c**
This is a simple example of a program which uses the *libbitmap* library. It generates a 100x100 colored `.bmp` file with user-defined name, depicting a blue circle of given radius. When you execute it, pass the two arguments (file name and radius value) along. Execution example: ```./bin/circle out/test.bmp 20```.

