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
int pipe_des1[2];
int pipe_des2[2];
int fd[2];
FILE *p_p;
FILE *p_h;
int cont = 0;
void manejador_hijo(int sig){//Aqui se ejecutan los comandos
    if(sig == SIGUSR1){
      if(execvp(line->commands[0].argv[0], line->commands[0].argv)< 0){
          char buff[1024];
          char *salida = "No se ha encontrado el mandato\n";
          strcpy(buff, salida);
          fputs(buff, stderr);
        }
    }else if(sig == SIGUSR2){
        if(cont != line->ncommands-1){
            close(pipe_des1[1]);
            close(pipe_des2[0]);
            close(STDIN_FILENO);
            dup(pipe_des2[0]);
            close(STDOUT_FILENO);
            dup(pipe_des1[1]);

            if(execvp(line->commands[0].argv[0], line->commands[0].argv)< 0){
                printf("Error al ejecutar el comando %s\n", line->commands[cont]);
            }

            kill(SIGUSR2, hijos[cont+1]);
        }


    }
      exit(0);
}

int main(void) {

    signal(SIGUSR1, manejador_hijo);
    signal(SIGUSR2, manejador_hijo);
	printf("msh> ");
	while (fgets(buf, 1024, stdin)) {
        line = tokenize(buf);
        pipe(fd);
        pid = fork();
        if (line==NULL) {
    		continue;
    	}

    	if (line->redirect_input != NULL) {
            if(pid == 0){
                printf("redirección de entrada: %s\n", line->redirect_input);
                close(fd[1]);
                close(STDIN_FILENO);
                p_p = fopen(line->redirect_input, "r");
                dup2(p_p, fd[0]);
                close(fd[0]);
            }
        }

    	if (line->redirect_output != NULL) {
            if(pid == 0){
                printf("redireccion de salida: %s\n", line->redirect_output);
                close(fd[0]);
                close(STDOUT_FILENO);
                p_p = fopen(line->redirect_output, "w");
                dup2(p_p, fd[1]);
                close(fd[1]);
            }
        }

        if (line->redirect_error != NULL) {
            if(pid == 0){
                printf("redireccion de error: %s\n", line->redirect_error);
                close(fd[0]);
                close(STDERR_FILENO);
                p_p = fopen(line->redirect_error, "w");
                dup2(p_p, fd[1]);

                close(fd[1]);
            }
        }

		if (line->background) {
			printf("comando a ejecutarse en background\n");
		}

        if(pid == 0){
            while(1);
        }else{
            for (i=0; i<line->ncommands; i++) {
                printf("orden %d (%s):\n", i, line->commands[i].filename);
                for (j=0; j<line->commands[i].argc; j++) {
                  printf("  argumento %d: %s\n", j, line->commands[i].argv[j]);
                }
            }
            printf("%d\n", line->ncommands);
            sleep(0.5);
            kill(pid, SIGUSR1);
            wait(pid);
        }

        printf("msh> ");

	}

	return 0;
}
