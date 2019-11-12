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
	char *salida;
	char buff[1024];

    typedef struct{
        char nombre[1024];
        int n;
        int pid;
		int status;  // 0 Running, 1 Done
    } tProcesoBackground;

    //tProcesoBackground procesosBackground[10]; // hay que hacerlo dinamico tb
	tProcesoBackground **procesosBackground;
	tProcesoBackground *procesoBackground;
    int contadorProcesosBackground = 0;

	float n = 100000;
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




	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	printf("msh> ");
	while (fgets(buf, 1024, stdin)) {



        line = tokenize(buf);

		if (line==NULL) {
		  continue;
		}

		if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"exit")==0){
			break;
		}

		for(int a = 0; a<contadorProcesosBackground; a++){
			if(procesosBackground[a]->status == 0){
				if(waitpid(procesosBackground[a]->pid, &status, WNOHANG) != 0){
					if(WIFEXITED(status) != 0){
						procesosBackground[a]->status = 1;
						if(WEXITSTATUS(status) != 0){
							procesosBackground[a]->pid = -1;
							strcpy(procesosBackground[a]->nombre, "");
							procesosBackground[a]->n = -1;
							procesosBackground[a]->status = -1;
							free(procesosBackground[a]);
							if(contadorProcesosBackground>0)
								contadorProcesosBackground--;
						}
					}
				}
			}
		}



		if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"jobs")==0){
			for(int a = 0; a<contadorProcesosBackground; a++){
				if(procesosBackground[a]->status == 0){
					printf("[%d]+   Running    %s\n", procesosBackground[a]->n, procesosBackground[a]->nombre);
				}else if(procesosBackground[a]->status == 1){
					printf("[%d]+   Done       %s\n", procesosBackground[a]->n, procesosBackground[a]->nombre);
					//AQUI SE ELIMINARIA DE LA LISTA EN DINAMICO, PONGO STATUS A -1 PARA QUE NO SALGA EN OTRO JOBS
					procesosBackground[a]->status = -1;
					free(procesosBackground[a]);
				}
			}
			printf("msh> ");
			continue;
		}

		if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"fg")==0){
			if(!(line->commands[0].argv[1] == NULL)){
				int procesoBg = atoi(line->commands[0].argv[1]);
				if(!(procesoBg < 1 || procesoBg > contadorProcesosBackground)){
					waitpid(procesosBackground[procesoBg-1]->pid, NULL, 0);
					procesosBackground[contadorProcesosBackground]->status = 1;
				}else{
					printf("myshell: fg: %d: no existe ese trabajo\n", procesoBg);
				}
			}else{
				waitpid(procesosBackground[contadorProcesosBackground-1]->pid, NULL, 0);
				procesosBackground[contadorProcesosBackground-1]->status = 1;
			}
			printf("msh> ");
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
				  fprintf(stderr, "Error: %s", strerror(errno));
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
					if(open(line->redirect_input, O_RDONLY) < 0){
						salida = line->redirect_input;
						strcat(salida, ": Error.");
						strcat(salida, strerror(errno));
						strcat(salida, "\n");
                        strcpy(buff, salida);
                        fputs(buff, stderr);
						exit(1);
					}else{
						p_h = open(line->redirect_input, O_RDONLY);
					}

                    dup2(p_h, fd[0]);
                    close(fd[0]);
                }
            }

          	if (line->redirect_output != NULL) {
                if(pid == 0){
                    printf("redireccion de salida: %s\n", line->redirect_output);
                    close(fd[0]);
                    close(STDOUT_FILENO);
					if(open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0600) < 0){
						salida = line->redirect_output;
						strcat(salida, ": Error.");
						strcat(salida, strerror(errno));
						strcat(salida, "\n");
                        strcpy(buff, salida);
                        fputs(buff, stderr);
						exit(1);
					}else{
							p_h = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0600);
					}

                    dup2(p_h, 1);
                    close(fd[1]);
                }
            }

            if (line->redirect_error != NULL) {
                if(pid == 0){
                    printf("redireccion de error: %s\n", line->redirect_error);
                    close(fd[0]);
                    close(STDERR_FILENO);
                    if(open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0600) < 0){
						salida = line->redirect_error;
						strcat(salida, ": Error.");
						strcat(salida, strerror(errno));
						strcat(salida, "\n");
                        strcpy(buff, salida);
                        fputs(buff, stderr);
						exit(1);
					}else{
						p_h = open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0600);
					}
                    dup2(p_h, fd[1]);
                    close(fd[1]);
                }
            }

      		if (line->background) {
				if(contadorProcesosBackground == 0){
					procesoBackground = (tProcesoBackground *) malloc(sizeof(tProcesoBackground));
					procesosBackground = (tProcesoBackground **) malloc(sizeof(tProcesoBackground*));
				}
				else{
					procesoBackground = (tProcesoBackground *) malloc(sizeof(tProcesoBackground));
					//tProcesoBackground **tmp = (tProcesoBackground **) realloc(procesosBackground, (contadorProcesosBackground+1) * (sizeof(tProcesoBackground*) + sizeof(tProcesoBackground)));
					tProcesoBackground **tmp = (tProcesoBackground **) realloc(procesosBackground, (contadorProcesosBackground+1) * sizeof(tProcesoBackground*));
					procesosBackground = tmp;

				}
				procesosBackground[contadorProcesosBackground] = procesoBackground;
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
					signal(SIGINT, SIG_IGN);
					signal(SIGQUIT, SIG_IGN);
                    if(execvp(line->commands[0].argv[0], line->commands[0].argv) < 0){

						salida = line->commands[0].argv[0];
						strcat(salida, ": No se ha encontrado el mandato.\n");
                        strcpy(buff, salida);
                        fputs(buff, stderr);
						exit(1);
                    }
                }else{
					contadorProcesosBackground++;
					procesosBackground[contadorProcesosBackground-1]->pid = pid;
					strcpy(procesosBackground[contadorProcesosBackground-1]->nombre, buff);
					procesosBackground[contadorProcesosBackground-1]->n = contadorProcesosBackground;
					procesosBackground[contadorProcesosBackground-1]->status = 0;
                    printf("[%d] %d\n", contadorProcesosBackground , procesosBackground[contadorProcesosBackground-1]->pid);
					sleep(1);
                }
            }else{
                if(pid < 0){
                    fprintf(stderr, "Error en la creacion del proceso hijo.\n");
                }else if(pid == 0){
					signal(SIGINT, SIG_DFL);
					signal(SIGQUIT, SIG_DFL);

                    if(execvp(line->commands[0].argv[0], line->commands[0].argv)< 0){

						salida = line->commands[0].argv[0];
						strcat(salida, ": No se ha encontrado el mandato.\n");
                        strcpy(buff, salida);
                        fputs(buff, stderr);
						exit(1);
                    }
                }else{
                    waitpid(pid, &status, 0);
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
							if(open(line->redirect_input, O_RDONLY) < 0){
								salida = line->redirect_input;
								strcat(salida, ": Error.");
								strcat(salida, strerror(errno));
								strcat(salida, "\n");
		                        strcpy(buff, salida);
		                        fputs(buff, stderr);
								exit(1);
							}else{
								p_h = open(line->redirect_input, O_RDONLY);
							}

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
							if(open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0600) < 0){
								salida = line->redirect_output;
								strcat(salida, ": Error.");
								strcat(salida, strerror(errno));
								strcat(salida, "\n");
		                        strcpy(buff, salida);
		                        fputs(buff, stderr);
								exit(1);
							}else{
								p_h = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0600);
							}

                            dup2(p_h, fd[1]);
                            close(fd[1]);
                        }
                        if (line->redirect_error != NULL) {
                            printf("redireccion de error: %s\n", line->redirect_error);
                            close(fd[0]);
                            close(STDERR_FILENO);
							if(open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0600) < 0){
								salida = line->redirect_error;
								strcat(salida, ": Error.");
								strcat(salida, strerror(errno));
								strcat(salida, "\n");
		                        strcpy(buff, salida);
		                        fputs(buff, stderr);
								exit(1);
							}else{
								p_h = open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0600);
							}

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
					signal(SIGINT, SIG_DFL);
					signal(SIGQUIT, SIG_DFL);


                    if (execv(line->commands[i].filename, line->commands[i].argv) < 0){

						salida = line->commands[i].argv[0];
						strcat(salida, ": No se ha encontrado el mandato.\n");
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
                waitpid(hijos[k], &status, 0);

            }

            free(pipes);
            free(hijos);
        }


		if(WIFSIGNALED(status)){
			printf("\n");
		}
		status = 0;
        printf("msh> ");
    }
	return 0;
}
