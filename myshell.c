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
	char buffJobs[2048];
	tline * line;
	int i;
	int pid;
	int fd[2];
	FILE *archivo;
	int jobsFile;
	int fichero;
	int p_h;
	int *hijos;
	int **pipes;
	int status;
	char *dir;
	char *salida;
	char salidaCd[1024];
	char salidaJobs[1024];
	char buff[1024];
	int error_bg;
	int a;
	int b;
	int c;

    typedef struct{
        char nombre[1024];
        int n;
		int status;  // 0 Running, 1 Done
		int n_mandatos;
		int * pids;
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

		// si la línea es vacía vuelve a esperar otra
		if (line==NULL) {
		  continue;
		}

		// salir de la minishell
		if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"exit")==0){
			break;
		}

		// comprobación con waitpid(WNOHANG) para comprobar los hijos que hayan muerto en bg y que no se queden zombies,
		// además de comprobación de errores al haber ejecutado mandatos en bg
		for( a = 0; a<contadorProcesosBackground; a++){
			if(procesosBackground[a]->status == 0){
				b = 0;
				error_bg = 0;
				while(b<procesosBackground[a]->n_mandatos && error_bg == 0){
					//printf("Proceso %d\n", procesosBackground[a]->pids[b]);
					if(waitpid(procesosBackground[a]->pids[b], &status, WNOHANG) != 0){
						//printf("A\n");
						if(WIFEXITED(status) != 0){
							//printf("B\n");
							if(WEXITSTATUS(status) != 0){	// para mandatos que salgan con error o no existan
								//printf("C\n");
								error_bg = 1;	// true
								procesosBackground[a]->status = -1;
								for( c = a; c<contadorProcesosBackground-1; c++){
									procesosBackground[c] = procesosBackground[c+1];
								}
								free(procesosBackground[contadorProcesosBackground-1]);
								if(contadorProcesosBackground>0)
									contadorProcesosBackground--;
							}else{
								if(b == procesosBackground[a]->n_mandatos - 1)
									procesosBackground[a]->status = 1;
							}
						}else{	// para mandatos que no han sido encontrados
							//printf("D\n");
							error_bg = 1;	// true
							procesosBackground[a]->status = -1;
							for( c = a; c<contadorProcesosBackground-1; c++){
								procesosBackground[c] = procesosBackground[c+1];
							}
							free(procesosBackground[contadorProcesosBackground-1]);
							if(contadorProcesosBackground>0)
								contadorProcesosBackground--;
						}
					}
					b++;
				}
				for( c = 0; c<procesosBackground[a]->n_mandatos; c++){
					waitpid(procesosBackground[a]->pids[c], NULL, WNOHANG);
				}
			}
		}

		// jobs
		if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"jobs")==0){
			if (line->redirect_output != NULL) {
                printf("redireccion de salida: %s\n", line->redirect_output);
				if(open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0600) < 0){
					salida = line->redirect_output;
					strcat(salidaJobs, ": Error.");
					strcat(salidaJobs, strerror(errno));
					strcat(salidaJobs, "\n");
                    strcpy(buff, salidaJobs);
                    fputs(buff, stderr);
					//continue;
				}else{

						char auxbuf[2048];
						strcpy(buffJobs, "");
						for( a = 0; a<contadorProcesosBackground; a++){
							if(procesosBackground[a]->status == 0){
								sprintf(auxbuf, "[%d]+   Running    %s", procesosBackground[a]->n, procesosBackground[a]->nombre);
								strcat(buffJobs, auxbuf);
							}else if(procesosBackground[a]->status == 1){
								sprintf(auxbuf,"[%d]+   Done       %s", procesosBackground[a]->n, procesosBackground[a]->nombre);
								strcat(buffJobs, auxbuf);

								//procesosBackground[a]->status = -1;
								// se libera de memoria dinamica a este proceso ya que se ha indicado
								for( c = a; c<contadorProcesosBackground-1; c++){
									procesosBackground[c] = procesosBackground[c+1];
								}
								free(procesosBackground[a]);
								if(contadorProcesosBackground>0)
									contadorProcesosBackground--;
							}
						}
						jobsFile = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0600);
						write(jobsFile, buffJobs, 2048);
				}

            }else{
				for( a = 0; a<contadorProcesosBackground; a++){
					if(procesosBackground[a]->status == 0){
						printf("[%d]+   Running    %s", procesosBackground[a]->n, procesosBackground[a]->nombre);
					}else if(procesosBackground[a]->status == 1){
						printf("[%d]+   Done       %s", procesosBackground[a]->n, procesosBackground[a]->nombre);
						//AQUI SE ELIMINARIA DE LA LISTA EN DINAMICO, PONGO STATUS A -1 PARA QUE NO SALGA EN OTRO JOBS
						procesosBackground[a]->status = -1;
						free(procesosBackground[a]);
					}
				}
			}


			printf("msh> ");
			continue;
		}

		// fg
		if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"fg")==0){
			if(contadorProcesosBackground > 0){
				if(!(line->commands[0].argv[1] == NULL)){
					int procesoBg = atoi(line->commands[0].argv[1]);
					if(procesoBg > 0 && procesoBg <= contadorProcesosBackground){
						for( a = 0; a<procesosBackground[procesoBg-1]->n_mandatos; a++){
							printf("%s\n", procesosBackground[procesoBg-1]->nombre);
							waitpid(procesosBackground[procesoBg-1]->pids[a], NULL, 0);
						}
						// aqui se pone como status al procesoBG en 1 (Done) ya que le hemos esperado en el wait,
						// por lo que ha terminado
						procesosBackground[procesoBg-1]->status = 1;
					}else{
						printf("myshell: fg: %d: no existe ese trabajo\n", procesoBg);
					}
				}else{
					for( a = 0; a<procesosBackground[contadorProcesosBackground-1]->n_mandatos; a++){
						waitpid(procesosBackground[contadorProcesosBackground-1]->pids[a], NULL, 0);
					}
					procesosBackground[contadorProcesosBackground-1]->status = 1;
				}
			}
			printf("msh> ");
			continue;
		}

		// cd
    	if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"cd")==0){
			if (line->redirect_input != NULL){
				printf("redirección de entrada: %s\n", line->redirect_input);
				if(open(line->redirect_input, O_RDONLY) < 0){
					strcpy(salidaCd, line->redirect_input);
					strcat(salidaCd, ": Error.");
					strcat(salidaCd, strerror(errno));
					strcat(salidaCd, "\n");
					fprintf(stderr,"%s\n", salidaCd);

				}else{
					archivo = fopen(line->redirect_input, "r");
					fscanf(archivo, "%s", buf2);
					if(chdir(buf2) != 0){
					  fprintf(stderr, "Error: %s\n", strerror(errno));
					}
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

    	}else if(line->ncommands == 1){			//Caso de que solo haya un mandato
            pipe(fd);	// pipe para redirecciones
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
				// reserva de memoria para almacenar los procesos en background
				if(contadorProcesosBackground == 0){
					procesoBackground = (tProcesoBackground *) malloc(sizeof(tProcesoBackground));
					procesosBackground = (tProcesoBackground **) malloc(sizeof(tProcesoBackground*));
				}
				else{
					procesoBackground = (tProcesoBackground *) malloc(line->ncommands * sizeof(int) + sizeof(tProcesoBackground));
					tProcesoBackground **tmp = (tProcesoBackground **) realloc(procesosBackground, (contadorProcesosBackground+1) * sizeof(tProcesoBackground*));
					procesosBackground = tmp;
				}
				procesosBackground[contadorProcesosBackground] = procesoBackground;
                if(pid < 0){
                    fprintf(stderr, "Error en la creacion del proceso hijo.\n");
                }else if(pid == 0){
					//signal(SIGINT, SIG_DFL);
					//signal(SIGQUIT, SIG_DFL);
                    if(execvp(line->commands[0].argv[0], line->commands[0].argv) < 0){
						fprintf(stderr, "%s: No se ha encontrado el mandato.\n", line->commands[0].argv[0]);
						exit(1);
                    }
                }else{
					procesosBackground[contadorProcesosBackground]->pids = (int *) malloc(sizeof(int));
					procesosBackground[contadorProcesosBackground]->pids[0] = pid;
					strcpy(procesosBackground[contadorProcesosBackground]->nombre, buf);
					procesosBackground[contadorProcesosBackground]->n = contadorProcesosBackground;
					procesosBackground[contadorProcesosBackground]->status = 0;
					procesosBackground[contadorProcesosBackground]->n_mandatos = 1;
					contadorProcesosBackground++;
                    fprintf(stdin, "[%d] %d\n", contadorProcesosBackground , procesosBackground[contadorProcesosBackground-1]->pids[0]);
					sleep(1);
                }
            }else{
                if(pid < 0){
                    fprintf(stderr, "Error en la creacion del proceso hijo.\n");
                }else if(pid == 0){
					signal(SIGINT, SIG_DFL);
					signal(SIGQUIT, SIG_DFL);
                    if(execvp(line->commands[0].argv[0], line->commands[0].argv)< 0){
						fprintf(stderr, "%s: No se ha encontrado el mandato.\n", line->commands[0].argv[0]);
						exit(1);
                    }
                }else{
                    waitpid(pid, &status, 0);
                }
            }

        }else if(line->ncommands >= 2){			//Caso de que haya mas de un mandato
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
					// si es el primer mandato
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

						// se cierran todos los pipes menos los primeros
                        for( c = 1; c < line->ncommands-1; c++){
                            close(pipes[c][0]);
                            close(pipes[c][1]);
                        }

						// cerramos el pipe que no se usa y la salida estandar y la sustituimos por el pipe
                        close(pipes[0][0]);
                        close(STDOUT_FILENO);
                        dup(pipes[0][1]);
                    }

					// si es un mandato que no es el primero ni el ultimo
                    if (i > 0 && i < line->ncommands-1){
						// cerramos todos los pipes menos los que se van a utilizar
                        for(c = 0; c < line->ncommands-1; c++){
                            if(c != i && c != i-1){
                                close(pipes[c][0]);
                                close(pipes[c][1]);
                            }
                        }
						// cerramos el pipe que no se usa y la entrada estandar y la sustituimos por el pipe
                        close(pipes[i-1][1]);
                        close(STDIN_FILENO);
                        dup(pipes[i-1][0]);

						// cerramos el pipe que no se usa y la salida estandar y la sustituimos por el pipe
                        close(pipes[i][0]);
                        close(STDOUT_FILENO);
                        dup(pipes[i][1]);
                    }

					// si es el último mandato
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

						// cerramos todos los pipes menos el último
                        for( c = 0; c<line->ncommands-2; c++){
                            close(pipes[c][0]);
                            close(pipes[c][1]);
                        }

						// cerramos el pipe que no se usa y la entrada estandar y la sustituimos por el pipe
                        close(pipes[i-1][1]);
                        close(STDIN_FILENO);
                        dup(pipes[i-1][0]);
                    }
					signal(SIGINT, SIG_DFL);
					signal(SIGQUIT, SIG_DFL);

					// se ejecuta el mandato correspondiente
                    if (execv(line->commands[i].filename, line->commands[i].argv) < 0){
						fprintf(stderr, "%s: No se ha encontrado el mandato.\n", line->commands[0].argv[0]);
						exit(1);
        			}
                }else{
						// se añaden todos los pids a hijos, para poder esperarlos después
						hijos[i] = pid;
				}

            } //for

			if(line->background){
				signal(SIGINT, SIG_IGN);
				signal(SIGQUIT, SIG_IGN);
				// reserva de memoria para almacenar los procesos en background
				if(contadorProcesosBackground == 0){
					procesoBackground = (tProcesoBackground *) malloc(line->ncommands * sizeof(int) + sizeof(tProcesoBackground));
					procesosBackground = (tProcesoBackground **) malloc(sizeof(tProcesoBackground*));
				}
				else{
					// REVISAR ESTA LINEA
					procesoBackground = (tProcesoBackground *) malloc(line->ncommands * sizeof(int) + sizeof(tProcesoBackground));
					tProcesoBackground **tmp = (tProcesoBackground **) realloc(procesosBackground, (contadorProcesosBackground+1) * sizeof(tProcesoBackground*));
					procesosBackground = tmp;
				}
				// se asignan los datos correspondientes a la estructura procesoBackground para poder gestionarla después
				procesosBackground[contadorProcesosBackground] = procesoBackground;
				strcpy(procesosBackground[contadorProcesosBackground]->nombre, buf);
				procesosBackground[contadorProcesosBackground]->n = contadorProcesosBackground;
				procesosBackground[contadorProcesosBackground]->status = 0;
				procesosBackground[contadorProcesosBackground]->n_mandatos = line->ncommands;
				procesosBackground[contadorProcesosBackground]->pids = (int *) malloc(line->ncommands * sizeof(int));
				contadorProcesosBackground++;
				for( a = 0; a<line->ncommands; a++){
					procesosBackground[contadorProcesosBackground]->pids[a] = hijos[a];
				}
				// se cierran los pipes en el proceso padre
				for( a = 0; a<line->ncommands-1; a++){
	                close(pipes[a][1]);
	                close(pipes[a][0]);
	            }
				// se imprime el numero del mandato y el pid del ultimo de los procesos
				printf("[%d] %d\n", contadorProcesosBackground , procesosBackground[contadorProcesosBackground-1]->pids[line->ncommands-1]);
				// sleep de 1 segundo por estética. Algunos procesos en background son rápidos
				// en ejecutarse. Para que no salga el prompt en medio.
				sleep(1);
			}else{	// si no es background
				// se cierran los pipes en el proceso padre
				for( a = 0; a<line->ncommands-1; a++){
	                close(pipes[a][1]);
	                close(pipes[a][0]);
	            }
				// se espera por cada uno de los hijos
				for(int k = 0; k<line->ncommands; k++){
	                waitpid(hijos[k], &status, 0);
	            }
            }

			// se libera en memoria los pipes y los pids de los hijos
            free(pipes);
            free(hijos);
        }

		// Por estética, para que salga el prompt en una nueva línea si
		// un proceso ha sido interrumpido por una señal como CTRL + C
		if(WIFSIGNALED(status)){
			printf("\n");
		}
        printf("msh> ");
    }
	return 0;
}
