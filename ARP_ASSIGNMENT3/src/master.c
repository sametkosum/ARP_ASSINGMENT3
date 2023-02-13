#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define KONSOLE "/usr/bin/konsole"

int spawn(const char * program, char * arg_list[])
{
  pid_t child_pid = fork();

  if (child_pid < 0)
  {
    perror("Error while forking...");
    return 1;
  }
  else if (child_pid != 0)
    return child_pid;
  else
  {
    if (execvp (program, arg_list) == 0);
    perror("Exec failed");
    return 1;
  }
}

int verify_numberOfport(char* numberOfport)
{
  while (*numberOfport)
  {
    if (isdigit(*numberOfport) == 0)
      return 1;
  }

  return 0;
}

int verify_address(char *IP_ADRESS)
{
  if (IP_ADRESS == "")
    return 1;

  int dots = 0;
  while (*IP_ADRESS)
  {
    if (*IP_ADRESS == '.')
      dots++;
    else if (isdigit(*IP_ADRESS) == 0)
      return 1;
    IP_ADRESS++;
  }

  if (dots != 3)
    return 1;

  return 0;
}

int main()
{
  printf("Welcome the Assingment 3(Advanced and Robot Programing)\n");
  printf("There are 3 modes selections:\n1) Normal\n2) Server\n3) Client\n");

  int mode_choice;
  do
  {
    printf("Could you select a modal: ");
    scanf("%d", &mode_choice);
  } while (mode_choice < 1 || mode_choice > 3);

  char IP_ADRESS[13];
  char numberOfport[6];
  char process[21];

  switch (mode_choice)
  {
  case 1:
    sprintf(process, "./bin/processA");
    break;

  case 2:
    printf("Server mode\nİNSERT PORT: ");
    scanf("%s", numberOfport);
    sprintf(process, "./bin/processAserver");
    break;

  case 3:
    printf("Client mode\ninsert IP address and port using a space between them: ");
    scanf("%s %s", IP_ADRESS, numberOfport);
    sprintf(process, "./bin/processAclient");
    break;

  default:
    return EXIT_FAILURE;
    break;
  }
  char *arg_list_A[] = { KONSOLE, "-e", process, IP_ADRESS, numberOfport, NULL };
  char *arg_list_B[] = { KONSOLE, "-e", "./bin/processB", NULL };
  pid_t process_A_pid = spawn(KONSOLE, arg_list_A);
  pid_t process_B_pid = spawn(KONSOLE, arg_list_B);
  int exit_status;
  waitpid(process_A_pid, &exit_status, 0);
  waitpid(process_B_pid, &exit_status, 0);
  printf ("Main program exiting with exit_status %d\n", exit_status);
  return EXIT_SUCCESS;
}

