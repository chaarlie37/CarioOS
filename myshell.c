#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include "parser.h"
#include <sys/types.h>
#include <sys/wait.h>
char buf[1024];
tline * line;
int i,j;
int pid;
int pipe_des1[2];
int pipe_des2[2];
int fd[2];
FILE *p_p;
FILE *p_h;
int cont;

int hijos[5];
int pipes[10][2];

void manejador_hijo(int sig);

int main(void) {

    signal(SIGUSR1, manejador_hijo);
    signal(SIGUSR2, manejador_hijo);
	printf("1msh> ");
	//while (fgets(buf, 1024, stdin)) {
    if(fgets(buf, 1024, stdin)){
        line = tokenize(buf);
        pid = fork();
        pipe(fd);

        if (line==NULL) {
    		//continue;
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
            int pid2 = 0;
            int pid_hijo;
            for (i=0; i<line->ncommands; i++) {
                pipe(pipes[i]);
                pid2 = fork();
                if(pid2 > 0){
                    hijos[i] = pid;
                    sleep(1);
                }
                else{
                    if(i == 0){
                        if(line->ncommands > 1){
                            printf("Primero\n");
                            close(pipes[i][0]);
                            close(STDOUT_FILENO);
                            dup(pipes[i][1]);
                            close(pipes[i][1]);
                            for(int a = i+1; a<line->ncommands; a++){
                                close(pipes[a][1]);
                                close(pipes[a][0]);
                            }
                        }
                        if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
                            printf("Error en execvp.\n");
                            exit(1);
                        }

                    }
                    else if(i > 0  &&  i < line->ncommands - 1){
                        printf("Medio\n");
                        close(pipes[i-1][1]);
                        close(STDIN_FILENO);
                        dup2(pipes[i-1][0], 0);
                        close(pipes[i-1][0]);

                        close(pipes[i][0]);
                        close(STDOUT_FILENO);
                        dup2(pipes[i][1], 1);
                        close(pipes[i][1]);


                        for(int a = 0; a<i-1; a++){
                            close(pipes[a][1]);
                            close(pipes[a][0]);
                        }
                        for(int a = i+1; a<line->ncommands; a++){
                            close(pipes[a][1]);
                            close(pipes[a][0]);
                        }

                        if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
                            printf("Error en execvp.\n");
                            exit(1);
                        }

                    }
                    else{
                        printf("Ultimo\n");
                        close(pipes[i-1][1]);
                        close(STDIN_FILENO);
                        dup(pipes[i-1][0]);
                        close(pipes[i-1][0]);

                        for(int a = i; a>0; a--){
                            close(pipes[a][1]);
                            close(pipes[a][0]);
                        }


                        if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
                            printf("Error en execvp.\n");
                            exit(1);
                        }

                    }
                }
            }

            for (i=0; i<line->ncommands; i++) {
                if(pid2>0){
                    //kill(hijos[i], SIGUSR2);
                    //sleep(0.5);
                    //kill(hijos[i+1], SIGUSR2);
                }

            }




            /*
            if(line->ncommands == 1){
                printf("%d\n", line->ncommands);
                sleep(0.5);
                kill(hijos[0], SIGUSR1);
            }else if(line->ncommands > 1){
                cont = 0;
                kill(hijos[0], SIGUSR2);
            }
            */

        }
        wait(NULL);
        printf("2msh> ");

	}

	return 0;
}


        void manejador_hijo(int sig){//Aqui se ejecutan los comandos
            if(sig == SIGUSR2){
                if(i == 0){
                    if(line->ncommands > 1){
                        close(pipes[i][0]);
                        close(STDOUT_FILENO);
                        dup(pipes[i][1]);
                        close(pipes[i][1]);
                        for(int a = i+1; a<line->ncommands; a++){
                            close(pipes[a][1]);
                            close(pipes[a][0]);
                        }
                    }
                    if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
                        printf("Error en execvp.\n");
                        exit(1);
                    }

                }
                else if(i > 0  &&  i < line->ncommands - 1){

                    close(pipes[i-1][1]);
                    close(STDIN_FILENO);
                    dup(pipes[i-1][0]);
                    close(pipes[i-1][0]);

                    close(pipes[i][0]);
                    close(STDOUT_FILENO);
                    dup(pipes[i][1]);
                    close(pipes[i][1]);

                    for(int a = i+1; a<line->ncommands; a++){
                        close(pipes[a][1]);
                        close(pipes[a][0]);
                    }

                    if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
                        printf("Error en execvp.\n");
                        exit(1);
                    }

                }
                else{

                    close(pipes[i-1][1]);
                    close(STDIN_FILENO);
                    dup(pipes[i-1][0]);
                    close(pipes[i-1][0]);
                    close(pipes[i][1]);
                    close(pipes[i][0]);

                    for(int a = i+1; a<line->ncommands; a++){
                        close(pipes[a][1]);
                        close(pipes[a][0]);
                    }

                    if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
                        printf("Error en execvp.\n");
                        exit(1);
                    }

                }
            }
        exit(0);
    }
