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
pid_t pid1, pid2, pid3;
int pipe_des1[2];
int pipe_des2[2];
int fd[2];
FILE *p_p;
FILE *p_h;
int cont;

int pipe_h2_h3[2], pipe_h1_h2[2];

int status;
int hijo_terminado;

int hijos[5];

int pipes[10][2];

int fd1[2], fd2[2];

int pid_shell;

void manejador_hijo(int sig);

char *cmd1[] = {"ls", "-la", 0};
char *cmd2[] = {"tr", "d", "D", 0};
char *cmd3[] = {"tr", "c", "C", 0};

int main(void) {

    pid_shell = getpid();

    //signal(SIGUSR1, manejador_hijo);
    //signal(SIGUSR2, manejador_hijo);
	printf("msh> ");
	while (getpid() == pid_shell && fgets(buf, 1024, stdin)) {
        line = tokenize(buf);
        pipe(pipe_h2_h3);
        pipe(pipe_h1_h2);
        pid1 = fork();
        if(pid1 == 0){
            /*
            close(fd2[1]);
            close(fd2[0]);
            close(fd1[0]);
            close(STDOUT_FILENO);
            dup(fd1[1]);

            execvp(line->commands[0].argv[0], line->commands[0].argv);
            close(fd1[1]);
            */
            close(pipe_h2_h3[0]);
            close(pipe_h2_h3[1]);
            close(pipe_h1_h2[0]);
            close(STDOUT_FILENO);
            dup(pipe_h1_h2[1]);
            execvp(cmd1[0], cmd1);

            printf("Cosas\n");
            close(pipe_h1_h2[1]);
            exit(0);
        }else{
            pid2 = fork();
            if(pid2 == 0){
                /*
                close(fd1[1]);
                close(STDIN_FILENO);
                dup(fd1[0]);
                close(fd1[0]);


                close(fd2[0]);
                close(STDOUT_FILENO);
                dup(fd2[1]);


                execvp(line->commands[1].argv[0], line->commands[1].argv);
                close(fd2[1]);
                */
                close(pipe_h1_h2[1]);
                close(STDIN_FILENO);
                dup(pipe_h1_h2[0]);
                close(pipe_h2_h3[0]);
                close(STDOUT_FILENO);
                dup(pipe_h2_h3[1]);
                printf("Buenas, aqui el hijo 2.\n");
                execvp(cmd2[0], cmd2);
                exit(0);

            }else{

                pid3 = fork();
                if(pid3 == 0){
                    /*
                    close(fd1[1]);
                    close(fd1[0]);
                    close(fd2[1]);
                    close(STDIN_FILENO);
                    dup2(fd2[0], 0);

                    execvp(line->commands[2].argv[0], line->commands[2].argv);
                    close(fd2[0]);
                    */
                    close(pipe_h1_h2[0]);
                    close(pipe_h1_h2[1]);
                    close(pipe_h2_h3[1]);
                    close(STDIN_FILENO);
                    dup(pipe_h2_h3[0]);
                    execvp(cmd3[0], cmd3);
                    close(pipe_h2_h3[0]);
                }
                else{
                }
            }
        }











        /*
        for (i=0; i<line->ncommands; i++) {
            if(i != line->ncommands - 1){
                //pipe(pipes[i]);
            }
            printf("Voy a hacer el fork. Soy el proceso %d...\n", getpid());
            if(i == 0){
                pipe(fd1);
                printf("FD1\n");
            }else if (i == 2){
                pipe(fd2);
                printf("FD2\n");
            }else{
                pipe(fd1);
                pipe(fd2);
                printf("AMBOS\n");
            }
            pid = fork();
            if(pid == 0){
                printf("Hijo Proceso %d...\n", getpid());
                if(i == 0){
                    if(line->ncommands > 1){
                        //printf("Primerisimo\n" );
                        close(fd1[0]);
                        close(STDOUT_FILENO);
                        dup2(fd1[1], 1);
                        close(fd1[1]);
                        //printf("Primerisimo2\n" );
                    }
                    if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
                        printf("Error en execvp.\n");
                        exit(1);
                    }

                }
                else if(i > 0  &&  i < line->ncommands - 1){

                    //printf("Medio.\n" );

                    close(STDIN_FILENO);
                    close(STDOUT_FILENO);
                    close(fd1[1]);
                    close(fd2[0]);
                    dup2(fd1[0], 0);
                    dup2(fd2[1], 1);
                    close(fd1[0]);
                    close(fd2[0]);


                    if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
                        printf("Error en execvp.\n");
                        exit(1);
                    }

                }
                else{

                    //printf("FINAL\n");
                    close(fd2[1]);
                    close(STDIN_FILENO);
                    dup2(fd2[0], 0);
                    close(fd2[0]);

                    char buff[1024];
                    fgets(buff, 1024, stdin);
                    printf("Buffer fin: %s", buff);

                    printf("FIN Hijo Proceso %d...\n", getpid());

                    if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
                        printf("Error en execvp.\n");
                        exit(1);
                    }

                }
                pause();
            }
            else{

                hijos[i] = pid;
                //kill(hijos[i], SIGUSR1);
                //printf("Padre: %d\n", getpid());
                sleep(1);
            }

        }

*/
    wait(NULL);
    printf("msh> ");

	}

	return 0;
}

    //  ls -la | tr a A | tr c C | tr r R

void manejador_hijo(int sig){//Aqui se ejecutan los comandos

    if(sig == SIGUSR2){     // si el mandato i es impar
        printf("Mandato %d: %s\n", i, line->commands[i].argv[0]);
        puts("PRUEBA");
        if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
            printf("Error en execvp.\n");
            exit(1);
        }

    }else if(sig == SIGUSR1){

        if(i == 0){

            if(line->ncommands > 1){
                printf("Primerisimo\n" );
                close(pipes[i][0]);
                close(STDOUT_FILENO);
                dup2(pipes[i][1], 1);
                close(pipes[i][1]);
                printf("Primerisimo2\n" );
            }

            if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
                printf("Error en execvp.\n");
                exit(1);
            }

        }
        else if(i > 0  &&  i < line->ncommands - 1){

            printf("Medio.\n" );

            close(pipes[i-1][1]);
            close(STDIN_FILENO);
            dup2(pipes[i-1][0], 0);
            close(pipes[i-1][0]);

            close(pipes[i][0]);
            close(STDOUT_FILENO);
            dup2(pipes[i][1], 1);
            close(pipes[i][1]);


            if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
                printf("Error en execvp.\n");
                exit(1);
            }

        }
        else{



            close(pipes[i-1][1]);
            close(STDIN_FILENO);
            dup2(pipes[i-1][0], 0);
            close(pipes[i-1][0]);


            if(execvp(line->commands[i].argv[0], line->commands[i].argv)<0){
                printf("Error en execvp.\n");
                exit(1);
            }

        }
    }


}
