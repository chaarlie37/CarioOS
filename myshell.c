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
	char buf2[1024];
	tline * line;
	int i;
	int pid;
	int fd[2];
	FILE *archivo;
	int fichero;
	int p_h;
	int *hijos;
	int **pipes;
	int status;
	char *dir;

    typedef struct{
        char nombre[1024];
        int n;
        int pid;
		int status;  // 0 Running, 1 Done
    } tProcesoBackground;

    tProcesoBackground procesosBackground[10]; // hay que hacerlo dinamico tb
    int contadorProcesosBackground = 0;

	float n = 150000;
	printf("\n\n\n     ██████╗ █████╗ ██████╗ ██╗ ██████╗      ██████╗ ███████╗\n");
	usleep(n);
	printf("    ██╔════╝██╔══██╗██╔══██╗██║██╔═══██╗    ██╔═══██╗██╔════╝\n");
	usleep(n);
	printf("    ██║     ███████║██████╔╝██║██║   ██║    ██║   ██║███████╗\n");
	usleep(n);
	printf("    ██║     ██╔══██║██╔══██╗██║██║   ██║    ██║   ██║╚════██║\n");
	usleep(n);
	printf("    ╚██████╗██║  ██║██║  ██║██║╚██████╔╝    ╚██████╔╝███████║\n");
	usleep(n);
	printf("     ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝ ╚═════╝      ╚═════╝ ╚══════╝\n\n\n");
	usleep(n);
	printf("Práctica MINISHELL para Sistemas Operativos. Universidad Rey Juan Carlos.\n");
	printf("Hecho por Carlos Sánchez Muñoz y Mario Manzaneque Ruiz. Noviembre de 2019.\n\n\n");





	printf("msh> ");
	while (fgets(buf, 1024, stdin)) {
        line = tokenize(buf);

		for(int a = 0; a<contadorProcesosBackground; a++){
			printf("VER SI ESPERAR A LOS BG\n");
			/*
			kill(procesosBackground[a].pid, 0);
			strerror(errno);
			printf("ERROR: %d\n", errno);
			if(errno == ESRCH){
				printf("ERROR QUE ME INTERESA\n");

			}else{
			*/
				if(procesosBackground[a].status == 0){
					if(waitpid(procesosBackground[a].pid, &status, WNOHANG) < 0){
						procesosBackground[a].status = 1;
					}
					if(WIFEXITED(status) != 0){
						if(WEXITSTATUS(status) != 0){
							printf("HA DAO ERROR EL HIJO\n");
							procesosBackground[contadorProcesosBackground].pid = -1;
							strcpy(procesosBackground[contadorProcesosBackground].nombre, "");
							procesosBackground[contadorProcesosBackground].n = -1;
							procesosBackground[contadorProcesosBackground].status = -1;
							if(contadorProcesosBackground>0)
								contadorProcesosBackground--;
						}
					}
					/*
					if(status != NULL){
						if(WIFEXITED(status)){
							printf("STATUS: %s\n", WEXITSTATUS(status));
							procesosBackground[a].status = 1;		// PREGUNTAR ESTO
						}
					}
					*/
				//}
			}
		}

        if (line==NULL) {
          continue;
        }

		if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"exit")==0){
			break;
		}

		if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"jobs")==0){
			for(int a = 0; a<contadorProcesosBackground; a++){
				if(procesosBackground[a].status == 0){
					printf("[%d]+   Running    %s\n", procesosBackground[a].n, procesosBackground[a].nombre);
				}else if(procesosBackground[a].status == 1){
					printf("[%d]+   Done       %s\n", procesosBackground[a].n, procesosBackground[a].nombre);
					//AQUI SE ELIMINARIA DE LA LISTA EN DINAMICO, PONGO STATUS A -1 PARA QUE NO SALGA EN OTRO JOBS
					procesosBackground[a].status = -1;
				}
			}
			printf("msh> ");
			continue;
		}

		if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"fg")==0){
			int procesoBg = atoi(line->commands[0].argv[1]);
			waitpid(procesosBackground[procesoBg-1].pid, NULL, 0);
			continue;
		}

    	if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"cd")==0){
			if (line->redirect_input != NULL){
				printf("redirección de entrada: %s\n", line->redirect_input);
				archivo = fopen(line->redirect_input, "r");
				fscanf(archivo, "%s", buf2);
				if(chdir(buf2) != 0){
				  fprintf(stderr, "Error: %s\n", strerror(errno));
				}

			}else{


				if(line->commands[0].argc == 2){
					dir = line->commands[0].argv[1];

				}else{

					dir = getenv("HOME");
				}
				if(chdir(dir) != 0){
					if(line->redirect_error != NULL){
						fichero = open(line->redirect_error, O_WRONLY);
						dup2(fichero, 2);
					}
				  fprintf(stderr, "Error: %s\n", strerror(errno));
				}
			}

    	}else if(line->ncommands == 1){//Caso de que solo haya un mandato
            pipe(fd);
            pid = fork();

            if (line->redirect_input != NULL) {
                if(pid == 0){
                    printf("redirección de entrada: %s\n", line->redirect_input);
                    close(fd[1]);
                    close(STDIN_FILENO);
                    p_h = open(line->redirect_input, O_RDONLY);
                    dup2(p_h, fd[0]);
                    close(fd[0]);
                }
            }

          	if (line->redirect_output != NULL) {
                if(pid == 0){
                    printf("redireccion de salida: %s\n", line->redirect_output);
                    close(fd[0]);
                    close(STDOUT_FILENO);
                    p_h = open(line->redirect_output, O_RDONLY);
                    dup2(p_h, fd[1]);
                    close(fd[1]);
                }
            }

            if (line->redirect_error != NULL) {
                if(pid == 0){
                    printf("redireccion de error: %s\n", line->redirect_error);
                    close(fd[0]);
                    close(STDERR_FILENO);
                    p_h = open(line->redirect_error, O_WRONLY);
                    dup2(p_h, fd[1]);
                    close(fd[1]);
                }
            }

      		if (line->background) {
				char buff[1024];
				strcpy(buff, "");
				for(int a = 0; a < line->commands[0].argc; a++){
					strcat(buff, line->commands[0].argv[a]);
					strcat(buff, " ");
				}
				strcat(buff, " &");
                if(pid < 0){
                    fprintf(stderr, "Error en la creacion del proceso hijo.\n");
                }else if(pid == 0){
                    if(execvp(line->commands[0].argv[0], line->commands[0].argv) < 0){
                        char buff[1024];
                        char *salida = "No se ha encontrado el mandato.\n";
                        strcpy(buff, salida);
                        fputs(buff, stderr);
						exit(1);
                    }
                }else{
					contadorProcesosBackground++;
					procesosBackground[contadorProcesosBackground-1].pid = pid;
					strcpy(procesosBackground[contadorProcesosBackground-1].nombre, buff);
					procesosBackground[contadorProcesosBackground-1].n = contadorProcesosBackground;
					procesosBackground[contadorProcesosBackground-1].status = 0;
                    printf("[%d] %d\n", contadorProcesosBackground , procesosBackground[contadorProcesosBackground-1].pid);
					sleep(1);
                }
            }else{
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
                            p_h = open(line->redirect_input, O_RDONLY);
                            dup2(p_h, fd[0]);
                            close(fd[0]);
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

                        if (line->redirect_output != NULL) {
                            printf("redireccion de salida: %s\n", line->redirect_output);
                            close(fd[0]);
                            close(STDOUT_FILENO);
                            p_h = open(line->redirect_output, O_WRONLY);
                            dup2(p_h, fd[1]);
                            close(fd[1]);
                        }
                        if (line->redirect_error != NULL) {
                            printf("redireccion de error: %s\n", line->redirect_error);
                            close(fd[0]);
                            close(STDERR_FILENO);
                            p_h = open(line->redirect_error, O_WRONLY);
                            dup2(p_h, fd[1]);
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
