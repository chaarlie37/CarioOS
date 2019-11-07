#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include "parser.h"
char buf[1024];
tline * line;
int i,j;
int pid;
int pipe_des[2];
int fd[2];
FILE *p_p;
FILE *p_h;

void manejador_hijo(int sig){//Aqui se ejecutan los comandos


  if(execvp(line->commands[0].argv[0], line->commands[0].argv)< 0){
    printf("No se ha encontrado el mandato\n");
  }

  exit(0);
}

int main(void) {

  signal(SIGUSR1, manejador_hijo);
	printf("msh> ");
	while (fgets(buf, 1024, stdin)) {



		line = tokenize(buf);
		if (line==NULL) {
			continue;
		}
		if (line->redirect_input != NULL) {
      printf("redirección de entrada: %s\n", line->redirect_input);
      pid = fork();
      pipe(fd);

      if(pid == 0){
        close(fd[1]);
        close(STDIN_FILENO);
        p_p = fopen(line->redirect_input, "r");
        dup2(p_p, fd[0]);

        close(fd[0]);

      }else{
        sleep(3);
        kill(pid, SIGUSR1);
        wait(NULL);
      }

		}
		if (line->redirect_output != NULL) {
			printf("redirección de salida: %s\n", line->redirect_output);

		}
		if (line->redirect_error != NULL) {
			printf("redirección de error: %s\n", line->redirect_error);
		}
		if (line->background) {
			printf("comando a ejecutarse en background\n");
		}else{
      pid = fork();
      if(pid == 0){
        while(1);
      }else{
        kill(pid, SIGUSR1);
        wait(NULL);
      }
    }
		for (i=0; i<line->ncommands; i++) {
			printf("orden %d (%s):\n", i, line->commands[i].filename);
			for (j=0; j<line->commands[i].argc; j++) {
				printf("  argumento %d: %s\n", j, line->commands[i].argv[j]);
			}
		}




		printf("msh> ");
	}
	return 0;
}
