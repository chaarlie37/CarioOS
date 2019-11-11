#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include "parser.h"
#include <sys/types.h>
#include <sys/wait.h>


//TODO:
// -estructura de procesos para el jobs
// -Revisar mensajes y casos de error

int main(void) {
	char buf[1024];
	tline * line;
	int i;
	int pid;
	int fd[2];
	FILE *p_p;
	int *hijos;
	int **pipes;


	printf("msh> ");
	while (fgets(buf, 1024, stdin)) {
        line = tokenize(buf);

        if (line==NULL) {
          continue;
        }

    	if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"cd")==0){
    		if(chdir(line->commands[0].argv[1]) != 0){
    	      fprintf(stderr, "Error: %s\n", strerror(errno));
    	    }
    	}else if(line->ncommands == 1){//Caso de que solo haya un mandato
            pipe(fd);
            pid = fork();

            // REVISAR: ¿Después de este fork no iria el if pid == 0 antes para cambiar la redireccion del hijo y no del padre?

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

            if(pid < 0){
                fprintf(stderr, "Error en la creacion del proceso hijo.\n");
            }else if(pid == 0){
                if(execvp(line->commands[0].argv[0], line->commands[0].argv)< 0){
                    char buff[1024];
                    char *salida = "No se ha encontrado el mandato.\n";
                    strcpy(buff, salida);
                    fputs(buff, stderr);
                }
            }else{
                waitpid(pid, NULL, 0);
            }

        }else if(line->ncommands >= 2){//Caso de que haya mas de un mandato
            hijos = malloc(line->ncommands * sizeof(int));
            pipes = (int **) malloc((line->ncommands-1) * sizeof(int *));
            for (i=0; i<line->ncommands-1; i++){
                pipes[i] = (int *) malloc (2*sizeof(int));
                if(pipe(pipes[i]) < 0){
    				fprintf(stderr, "Error al crear el pipe %d.\n", i);
    			}
            }
            pipe(fd);
            for (i=0; i<line->ncommands; i++) {
                pid = fork();
                if(pid < 0){
                    fprintf(stderr, "Error al crear el hijo\n");
                }else if (pid == 0){
                    if(i == 0){
                        if (line->redirect_input != NULL) {
                            printf("redirección de entrada: %s\n", line->redirect_input);
                            close(fd[1]);
                            close(STDIN_FILENO);
                            p_p = fopen(line->redirect_input, "r");
                            dup2(p_p, fd[0]);
                            close(fd[0]);
                        }
                        if (line->redirect_output != NULL) {
                            char buff[1024];
                            char *salida = "No se puede hacer redireccion de salida en un comando que no sea el ultimo\n";
                            strcpy(buff, salida);
                            fputs(buff, stderr);
                            exit(1);
                        }

                        for(int c = 1; c < line->ncommands-1; c++){
                            close(pipes[c][0]);
                            close(pipes[c][1]);
                        }

                        close(pipes[0][0]);
                        close(STDOUT_FILENO);
                        dup(pipes[0][1]);
                    }

                    if (i > 0 && i < line->ncommands-1){
                        if (line->redirect_output != NULL) {
                            char buff[1024];
                            char *salida = "No se puede hacer redireccion de salida en un mandato que no sea el ultimo\n";
                            strcpy(buff, salida);
                            fputs(buff, stderr);
                            exit(1);
                        }
                        if (line->redirect_input != NULL) {
                            char buff[1024];
                            char *salida = "No se puede hacer redireccion de entrada en un mandato que no sea el primero\n";
                            strcpy(buff, salida);
                            fputs(buff, stderr);
                            exit(1);
                        }

                        for(int c = 0; c < line->ncommands-1; c++){
                            if(c != i && c != i-1){
                                close(pipes[c][0]);
                                close(pipes[c][1]);
                            }
                        }
                            close(pipes[i-1][1]);
                            close(STDIN_FILENO);
                            dup(pipes[i-1][0]);

                            close(pipes[i][0]);
                            close(STDOUT_FILENO);
                            dup(pipes[i][1]);
                    }

                    if(i == line->ncommands-1){
                        if (line->redirect_input != NULL) {
                            char buff[1024];
                            char *salida = "No se puede hacer redireccion de entrada en un mandato que no sea el primero\n";
                            strcpy(buff, salida);
                            fputs(buff, stderr);
                            exit(1);
                        }
                        if (line->redirect_output != NULL) {
                            printf("redireccion de salida: %s\n", line->redirect_output);
                            close(fd[0]);
                            close(STDOUT_FILENO);
                            p_p = fopen(line->redirect_output, "w");
                            dup2(p_p, fd[1]);
                            close(fd[1]);
                        }
                        if (line->redirect_error != NULL) {
                            printf("redireccion de error: %s\n", line->redirect_error);
                            close(fd[0]);
                            close(STDERR_FILENO);
                            p_p = fopen(line->redirect_error, "w");
                            dup2(p_p, fd[1]);
                            close(fd[1]);
                        }

                        for(int c = 0; c<line->ncommands-2; c++){
                            close(pipes[c][0]);
                            close(pipes[c][1]);
                        }

                        close(pipes[i-1][1]);
                        close(STDIN_FILENO);
                        dup(pipes[i-1][0]);
                    }

                    if (execv(line->commands[i].filename, line->commands[i].argv) < 0){
        				char buff[1024];
        				char *salida = "No se ha encontrado el mandato\n";
        				strcpy(buff, salida);
        				fputs(buff, stderr);
        				exit(1);
        			}
                }else{
                    hijos[i] = pid;
                }
            }

            for(int a = 0; a<line->ncommands-1; a++){
                close(pipes[a][1]);
                close(pipes[a][0]);
            }

            for(int k = 0; k<line->ncommands; k++){
                waitpid(hijos[k], NULL, 0);
            }

            free(pipes);
            free(hijos);
        }
        printf("msh> ");
    }
	return 0;
}
