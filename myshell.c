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

int pidFg;
void manejador(int sig);

void imprimirPrompt(){
	printf("\033[1;36m");
	printf("msh> ");
	printf("\033[0m");
}

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
	printf("\033[1;36m");


	printf("\n\n\n     ██████╗ █████╗ ██████╗ ██╗ ██████╗      ██████╗  ██████╗\n");
	usleep(n);
	printf("    ██╔════╝██╔══██╗██╔══██╗██║██╔═══██╗    ██╔═══██╗██╔════╝\n");
	usleep(n);
	printf("    ██║     ███████║██████╔╝██║██║   ██║    ██║   ██║███████╗\n");
	usleep(n);
	printf("    ██║     ██╔══██║██╔══██╗██║██║   ██║    ██║   ██║╚════██║\n");
	usleep(n);
	printf("    ╚██████╗██║  ██║██║  ██║██║╚██████╔╝    ╚██████╔╝██████╔╝ \n");
	usleep(n);
	printf("     ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝ ╚═════╝      ╚═════╝ ╚═════╝ \n\n\n");
	usleep(n);

	printf("\033[0m");
	printf("\033[1m");
	//printf("\033[1;37m");

	printf("Práctica MINISHELL para Sistemas Operativos. Universidad Rey Juan Carlos.\n");
	printf("Hecho por Carlos Sánchez Muñoz y Mario Manzaneque Ruiz. Noviembre de 2019.\n\n\n");

	printf("\033[0m");

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	imprimirPrompt();
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
					//		printf("B\n");
							if(WEXITSTATUS(status) != 0){	// para mandatos que salgan con error o no existan
					//			printf("C\n");
								error_bg = 1;	// true
								procesosBackground[a]->status = -1;
								for( c = a; c<contadorProcesosBackground-1; c++){
									procesosBackground[c] = procesosBackground[c+1];
								}
								free(procesosBackground[contadorProcesosBackground]);
								if(contadorProcesosBackground>0)
									contadorProcesosBackground--;
							}else{
								if(b == procesosBackground[a]->n_mandatos - 1){
									procesosBackground[a]->status = 1;
									printf("[%d]+   Done       %s", procesosBackground[a]->n, procesosBackground[a]->nombre);
									procesosBackground[a]->status = -1;
								}
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
						}
					}
					jobsFile = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0600);
					write(jobsFile, buffJobs, 2048);
				}
            }else{
				for( a = 0; a<contadorProcesosBackground; a++){
					if(procesosBackground[a]->status == 0){
						printf("[%d]+   Running    %s", procesosBackground[a]->n, procesosBackground[a]->nombre);
					}
				}
			}
			imprimirPrompt();
			continue;
		}

		// fg
		if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"fg")==0){
			signal(SIGINT, manejador);
			signal(SIGQUIT, manejador);
			if(contadorProcesosBackground > 0){
				if(!(line->commands[0].argv[1] == NULL)){
					int procesoBg = atoi(line->commands[0].argv[1]);
					if(procesoBg > 0 && procesoBg <= contadorProcesosBackground){
						for( a = 0; a<procesosBackground[procesoBg-1]->n_mandatos; a++){
							printf("%s\n", procesosBackground[procesoBg-1]->nombre);
							pidFg = procesosBackground[procesoBg-1]->pids[a];
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
						pidFg = procesosBackground[contadorProcesosBackground-1]->pids[a];
						waitpid(procesosBackground[contadorProcesosBackground-1]->pids[a], NULL, 0);
					}
					procesosBackground[contadorProcesosBackground-1]->status = 1;
				}
			}
			imprimirPrompt();
			continue;
		}

		// cd
    	if(line->ncommands == 1 && strcmp(line->commands[0].argv[0],"cd")==0){
			if (line->redirect_input != NULL){//caso de que haya una redireccion de entrada
				printf("redirección de entrada: %s\n", line->redirect_input);
				if(open(line->redirect_input, O_RDONLY) < 0){//se comprueba que no haya errores al abrir el fichero
					strcpy(salidaCd, line->redirect_input);
					strcat(salidaCd, ": Error.");
					strcat(salidaCd, strerror(errno));
					strcat(salidaCd, "\n");
					fprintf(stderr,"%s\n", salidaCd);

				}else{
					archivo = fopen(line->redirect_input, "r");
					fscanf(archivo, "%s", buf2);
					if(chdir(buf2) != 0){//se ejecuta el mandato cd usando el contenido del fichero
					  fprintf(stderr, "Error: %s\n", strerror(errno));
					}
				}


			}else{


				if(line->commands[0].argc == 2){//si se le introduce una ruta a la que ir
					dir = line->commands[0].argv[1];

				}else{
					//si no se introduce una ruta a la que ir, el mandato hace un cambio de directorio al contenido de la variable HOME
					dir = getenv("HOME");
				}
				if(chdir(dir) != 0){//se ejecuta el mandato con el contenido de la variable dir
					if(line->redirect_error != NULL){//casod de que haya una redireccion de la salida estandar de error
						if(open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0600) < 0){//comprobante de que haya algún fallo a la hora de abrir el fichero
							//En caso de que haya algun problema se contruye un mensaje de error
							printf("redireccion de error: %s\n", line->redirect_error);
							salida = line->redirect_error;
							strcat(salida, ": Error.");
							strcat(salida, strerror(errno));
							strcat(salida, "\n");
	                        strcpy(buff, salida);
	                        fputs(buff, stderr);
							exit(1);
						}else{
							fichero = open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0600);
							dup2(fichero, 2);
						}

					}
				  fprintf(stderr, "Error: %s\n", strerror(errno));
				}
			}

    	}else if(line->ncommands == 1){			//Caso de que solo haya un mandato
            pipe(fd);	// pipe para redirecciones
            pid = fork();
            if (line->redirect_input != NULL) {//caso de que haya una redireccion de entrada
                if(pid == 0){//el hijo
                    printf("redirección de entrada: %s\n", line->redirect_input);
                    close(fd[1]);
                    close(STDIN_FILENO);//se cierra la entrada estandar
					if(open(line->redirect_input, O_RDONLY) < 0){//comprobacion de fallo a la hora de abrir el fichero
						//en caso de que de algún problema se construye el mensaje de error
						salida = line->redirect_input;
						strcat(salida, ": Error.");
						strcat(salida, strerror(errno));
						strcat(salida, "\n");
                        strcpy(buff, salida);
                        fputs(buff, stderr);//se pone el mensaje de error en la salida estandar de error
						exit(1);
					}else{
						//en caso de que se abra el fichero de forma correcta
						p_h = open(line->redirect_input, O_RDONLY);//se abre el fichero y se guarda el descriptor en la variable p_h
					}

                    dup2(p_h, fd[0]);//se sustituye la entrada estandar por el contenido del fichero
                    close(fd[0]);
                }
            }

          	if (line->redirect_output != NULL) {//caso de que haya redireccion de salida
                if(pid == 0){//el hijo
                    printf("redireccion de salida: %s\n", line->redirect_output);
                    close(fd[0]);
                    close(STDOUT_FILENO);//se cierra la salida estandar
					if(open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0600) < 0){//comprobante de que haya algún fallo a la hora de abrir el fichero
						//En caso de que haya algun problema se contruye un mensaje de error
						salida = line->redirect_output;
						strcat(salida, ": Error.");
						strcat(salida, strerror(errno));
						strcat(salida, "\n");
                        strcpy(buff, salida);
                        fputs(buff, stderr);
						exit(1);
					}else{
						//si se ha podido abrir correctamente
						p_h = open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0600);
					}
                    dup2(p_h, 1);//se sustituye la salida estandar por el fichero
                    close(fd[1]);
                }
            }

            if (line->redirect_error != NULL) {//caso de que haya redireccion de salida de error
                if(pid == 0){//el hijo
                    printf("redireccion de error: %s\n", line->redirect_error);
                    close(fd[0]);
                    close(STDERR_FILENO);//se cierra la salida estandar de error
                    if(open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0600) < 0){//comprobante de que haya algún fallo a la hora de abrir el fichero
						//En caso de que haya algun problema se contruye un mensaje de error
						salida = line->redirect_error;
						strcat(salida, ": Error.");
						strcat(salida, strerror(errno));
						strcat(salida, "\n");
                        strcpy(buff, salida);
                        fputs(buff, stderr);
						exit(1);
					}else{
						//si se ha podido abrir correctamente
						p_h = open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0600);
					}
                    dup2(p_h, fd[1]);//se sustituye la salida de error por el fichero
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
					procesosBackground[contadorProcesosBackground]->n = contadorProcesosBackground + 1;
					procesosBackground[contadorProcesosBackground]->status = 0;
					procesosBackground[contadorProcesosBackground]->n_mandatos = 1;
					contadorProcesosBackground++;
                    fprintf(stdin, "[%d] %d\n", contadorProcesosBackground , procesosBackground[contadorProcesosBackground-1]->pids[0]);
					sleep(1);
                }
            }else{
                if(pid < 0){
                    fprintf(stderr, "Error en la creacion del proceso hijo.\n");
                }else if(pid == 0){//el hijo
					//se hace que vuelva a escuchar las señales que la shell tiene que ignorar (ctrl+c y ctrl+\)
					signal(SIGINT, SIG_DFL);
					signal(SIGQUIT, SIG_DFL);
                    if(execvp(line->commands[0].argv[0], line->commands[0].argv)< 0){//ejecución del mandato y comprobante de que todo vaya correctamente
						fprintf(stderr, "%s: No se ha encontrado el mandato.\n", line->commands[0].argv[0]);
						exit(1);
                    }
                }else{
					//el padre espera a que el hijo termine
                    waitpid(pid, &status, 0);
                }
            }

        }else if(line->ncommands >= 2){			//Caso de que haya mas de un mandato
            hijos = malloc(line->ncommands * sizeof(int));//se reserva memoria para los pid de los hijos
            pipes = (int **) malloc((line->ncommands-1) * sizeof(int *)); //se reserva memoria para los pipes necesarios
            for (i=0; i<line->ncommands-1; i++){
                pipes[i] = (int *) malloc (2*sizeof(int));//se reserva memoria dentro de cada uno de los pipes para lectura y escritura
                if(pipe(pipes[i]) < 0){//se abre cada uno de los pipes comprobando si hay fallos
    				fprintf(stderr, "Error al crear el pipe %d.\n", i);
    			}
            }
            pipe(fd);//pipe para redirecciones
            for (i=0; i<line->ncommands; i++) {
                pid = fork();
                if(pid < 0){
                    fprintf(stderr, "Error al crear el hijo\n");
                }else if (pid == 0){
					// si es el primer mandato
                    if(i == 0){
                        if (line->redirect_input != NULL) {//caso de que haya una redireccion de entrada
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

						// se cierran todos los pipes menos el primero
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
						// cerramos el extremo del pipe que no se usa y la entrada estandar y la sustituimos por el pipe
                        close(pipes[i-1][1]);
                        close(STDIN_FILENO);
                        dup(pipes[i-1][0]);

						// cerramos el extremo del pipe que no se usa y la salida estandar y la sustituimos por el pipe
                        close(pipes[i][0]);
                        close(STDOUT_FILENO);
                        dup(pipes[i][1]);
                    }

					// si es el último mandato
                    if(i == line->ncommands-1){

                        if (line->redirect_output != NULL) {//caso de que haya redireccion de salida
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
                        if (line->redirect_error != NULL) {//caso de que haya redireccion de salida de error
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
                    if (execvp(line->commands[i].filename, line->commands[i].argv) < 0){
						fprintf(stderr, "%s: No se ha encontrado el mandato.\n", line->commands[0].argv[0]);
						exit(1);
        			}
                }else{
						// se añaden todos los pids a hijos, para poder esperarlos después
						hijos[i] = pid;
				}

            } //for

			if(line->background){
				//se hace que los procesos en background ignoren las señales (ctrl+c y ctrl+\)
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
				procesosBackground[contadorProcesosBackground]->n = contadorProcesosBackground + 1;
				procesosBackground[contadorProcesosBackground]->status = 0;
				procesosBackground[contadorProcesosBackground]->n_mandatos = line->ncommands;
				procesosBackground[contadorProcesosBackground]->pids = (int *) malloc(line->ncommands * sizeof(int));

				for( a = 0; a<line->ncommands; a++){
					procesosBackground[contadorProcesosBackground]->pids[a] = hijos[a];
				}
				contadorProcesosBackground++;
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
				for(a = 0; a<line->ncommands; a++){
	                waitpid(hijos[a], &status, 0);
	            }
            }

			// se libera en memoria los pipes y los pids de los hijos
			for(a = 0; a<line->ncommands; a++){
				free(pipes[a]);
			}
            free(pipes);
            free(hijos);
        }

		// Por estética, para que salga el prompt en una nueva línea si
		// un proceso ha sido interrumpido por una señal como CTRL + C
		if(WIFSIGNALED(status)){
			printf("\n");
		}
        imprimirPrompt();
    }
	return 0;
}


void manejador(int sig){
	kill(pidFg, SIGKILL);
}
