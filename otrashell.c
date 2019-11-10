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

int status;
int hijo_terminado;

int hijos[5];
void manejador_hijo(int sig);

int main(void) {


    signal(SIGUSR1, manejador_hijo);
    signal(SIGUSR2, manejador_hijo);
	printf("msh> ");
	while (fgets(buf, 1024, stdin)) {
        line = tokenize(buf);
        pipe(pipe_des1);
        pipe(pipe_des2);
        for (i=0; i<line->ncommands; i++) {
            pid = fork();
            if(pid < 0){
                printf("Error en el fork.\n");
            }
            else if(pid == 0){
                printf("------------Hijo%d con su hijo %d\n", getpid(), pid);
                pause();
            }
            else{
                hijos[i] = pid;
            }


        }

        for (i=0; i<line->ncommands; i++) {
            if(pid > 0){
                if(i % 2 == 0){   //mandatos pares
                    printf("Pares %d\n", i);
                    kill(hijos[i], SIGUSR2);
                }
                else    // mandatos impares
                {
                    printf("Impares %d\n", i);
                    kill(hijos[i], SIGUSR1);
                }
                printf("\nHijo %d...\n", hijos[i]);

                printf("Voy a esperar a mi hijo %d %d\n", i, hijos[i] );
                hijo_terminado=waitpid(hijos[i], &status, 0);
                printf("\n---Hijo %d terminó. Status: %d\n", hijo_terminado, WIFEXITED(status));
            }
        }


        printf("msh> ");

        //wait(NULL);

        /*
        hijo_terminado=waitpid(hijos[i], &status, 0);
        printf("\n---Hijo %d terminó. Status: %d\n", hijo_terminado, WIFEXITED(status));
        */
	}

	return 0;
}

    //  ls -la | tr a A | tr c C | tr r R

void manejador_hijo(int sig){//Aqui se ejecutan los comandos

    if(sig == SIGUSR2){     // si el mandato i es impar
        if(line->ncommands == 1){ // si hay solo un mandato (no hay pipes)
          if(execvp(line->commands[0].argv[0], line->commands[0].argv)< 0){
              char buff[1024];
              char *salida = "No se ha encontrado el mandato\n";
              strcpy(buff, salida);
              fputs(buff, stderr);
            }
        }
        else
        {
            if(i == 0){

                printf("Primerisimo\n" );

                close(pipe_des1[0]);
                close(STDOUT_FILENO);
                dup(pipe_des1[1]);
                close(pipe_des1[1]);
                close(pipe_des2[1]);
                close(pipe_des2[0]);

                execvp(line->commands[i].argv[0], line->commands[i].argv);
            }
            else if(i>0 && i<line->ncommands-1)
            {

                printf("Par en el medio.\n" );

                close(pipe_des2[1]);
                close(STDIN_FILENO);
                dup(pipe_des2[0]);
                //close(pipe_des2[0]);

                close(pipe_des1[0]);
                close(STDOUT_FILENO);
                dup(pipe_des1[1]);
                //close(pipe_des1[1]);

                execvp(line->commands[i].argv[0], line->commands[i].argv);
            }
            else
            {
                printf("Par ultimo.\n" );

                close(pipe_des2[1]);
                close(STDIN_FILENO);
                dup(pipe_des2[0]);
                close(pipe_des2[0]);

                execvp(line->commands[i].argv[0], line->commands[i].argv);
            }

        }


    }
    else if(sig == SIGUSR1)
    {
        if(i>0 && i<line->ncommands-1)
        {
            printf("Impar en el medio.\n" );

            close(pipe_des1[1]);
            close(STDIN_FILENO);
            dup(pipe_des1[0]);
            close(pipe_des1[0]);

            close(pipe_des2[0]);        // pipe solo de escritura
            close(STDOUT_FILENO);
            dup(pipe_des2[1]);
            close(pipe_des2[1]);


            execvp(line->commands[i].argv[0], line->commands[i].argv);
        }
        else
        {

            printf("Impar ultimo.\n" );

            close(pipe_des1[1]);
            close(STDIN_FILENO);
            dup(pipe_des1[0]);
            close(pipe_des1[0]);


            execvp(line->commands[i].argv[0], line->commands[i].argv);

        }
    }


}
