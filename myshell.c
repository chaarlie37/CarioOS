#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "parser.h"

char buf[1024];
tline * line;
int i,j;
int pid;

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
			//printf("redirección de entrada: %s\n", line->redirect_input);
			
			pid = fork();
			if(pid == 0){

			}else{

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
		}
		for (i=0; i<line->ncommands; i++) {
			printf("orden %d (%s):\n", i, line->commands[i].filename);
			for (j=0; j<line->commands[i].argc; j++) {
				printf("  argumento %d: %s\n", j, line->commands[i].argv[j]);
			}
		}

    pid = fork();
    if(pid == 0){
      while(1);
    }else{
      kill(pid, SIGUSR1);
      wait(NULL);
    }

		printf("msh> ");
	}
	return 0;
}
