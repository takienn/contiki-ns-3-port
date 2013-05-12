#include <unistd.h>

int contiki_start(int argc, char **argv);

int main(int argc, char **argv){

  int sockets[2];
  
  pid_t pid;
  pid = fork();
  if(pid == -1)
    perror("fork() failed");
  else if (pid ==0){
    close(sockets[0]);
    dup2(sockets[1], STDIN_FILENO);
    //char addr[] = "-a:c";
    contiki_start(0,(char **)NULL);
  } 

  return 0;

}
