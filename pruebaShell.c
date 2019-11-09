#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include "parser.h"
char buf[1024];
char buf2[1024];
tline * line;
int i,j;
int pid1;
int pid2;
int pipe_des1[2];
int pipe_des2[2];
int fd[2];
FILE *p_p;
FILE *p_h;
FILE *lectura;
FILE *lectura2;
int cont;

int hijos[5];
void manejador_hijo(int sig);

int main(void) {

    signal(SIGUSR1, manejador_hijo);
    signal(SIGUSR2, manejador_hijo);
	printf("msh> ");
	while (fgets(buf, 1024, stdin)) {
        line = tokenize(buf);

        if (line==NULL) {
    		continue;
    	}
        pid2 = fork();




    	if (line->redirect_output != NULL) {
            if(pid == 0){
                printf("redireccion de salida: %s\n", line->redirect_output);
                close(fd[0]);
                close(STDOUT_FILENO);
                p_p = fopen(line->redirect_output, "w");
                dup2(p_p, fd[1]);
                kill(pid1 , SIGUSR2);
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
                kill(pid1, SIGUSR2);
                close(fd[1]);
            }
        }

		if (line->background) {
			printf("comando a ejecutarse en background\n");
		}

        if(pid2 == 0){
            while(1);
        }else{
            pipe(fd);
            pipe(pipe_des1);
            pipe(pipe_des2);
            for (i=0; i<line->ncommands; i++) {
                printf("orden %d (%s):\n", i, line->commands[i].filename);
                for (j=0; j<line->commands[i].argc; j++) {
                  printf("  argumento %d: %s\n", j, line->commands[i].argv[j]);
                }

                int pid2 = fork();
                if(pid2 == 0){
                    if(i % 2 == 0){   //mandatos pares
                        if(i == 0){
                            if (line->redirect_input != NULL) {
                                    printf("redirección de entrada: %s\n", line->redirect_input);
                                    close(fd[1]);
                                    close(STDIN_FILENO);
                                    p_p = fopen(line->redirect_input, "r");
                                    dup2(p_p, fd[0]);
                            }
                            close(pipe_des2[0]);
                            close(pipe_des2[1]);
                            close(pipe_des1[0]);
                            close(STDOUT_FILENO);
                            dup(pipe_des1[1]);
                            kill(hijos[i], SIGUSR2);

                            exit(0);
                        }else if (i > 0 && i < line->ncommands-1){
                            close(pipe_des2[1]);
                            close(pipe_des1[0]);
                            close(STDIN_FILENO);
                            dup(pipe_des2[0]);
                            close(STDOUT_FILENO);
                            dup(pipe_des1[1]);
                            kill(hijos[i], SIGUSR2);
                            exit(0);
                        }else{
                            close(pipe_des2[1]);
                            close(STDIN_FILENO);
                            dup(pipe_des2[0]);
                            kill(hijos[i], SIGUSR2);
                            exit(0);

                        }
                    }
                    else    // mandatos impares
                    {
                        if(i> 0 && i < line->ncommands-1){
                            close(pipe_des1[1]);
                            close(pipe_des2[0]);
                            close(STDIN_FILENO);
                            dup(pipe_des1[0]);
                            close(STDOUT_FILENO);
                            dup(pipe_des2[1]);
                            kill(hijos[i], SIGUSR2);
                            exit(0);
                        }else if (i == line->ncommands-1){
                            close(pipe_des1[1]);
                            close(STDIN_FILENO);
                            dup(pipe_des1[0]);
                            kill(hijos[i], SIGUSR2);
                            exit(0);

                        }

                    }
                }
                else{
                    hijos[i] = pid2;

                }
                if(i % 2 == 0){   //mandatos pares
                    printf("Pares %d\n", i);


                }
                else    // mandatos impares
                {
                    printf("Impares %d\n", i);


                }
                wait(hijos[i]);

            }



        }

        printf("msh> ");

	}

	return 0;
}

void manejador_hijo(int sig){
    if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
        printf("No se encuentra el comando\n");
    }
    exit(0);
}
