#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


#define COCHES 30
#define CAMIONES 3
#define PLAZAS 7
#define PLANTAS 3
int estado_coches[COCHES + CAMIONES];
int aparcamiento_plantas[PLANTAS][PLAZAS];
int aparcamiento[PLAZAS];

int plazas_libres = PLAZAS * PLANTAS;
pthread_mutex_t mutexGen;
pthread_mutex_t mutex[PLAZAS];
pthread_cond_t espera[COCHES];

void print_estado(){
    printf("Parking: \n");
    for(int i = 0; i<PLANTAS; i++){
        for(int j = 0; j<PLAZAS; j++){
            printf("[%d]", aparcamiento_plantas[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void *camionAparca(void *num){
    int coche_id = *(int *) num;
    int ocupado = 1;
    int i;
    //sleep(2);
    while(1){
        estado_coches[coche_id] = 0;
        pthread_mutex_lock(&mutexGen);
        if(plazas_libres > 1){

            for(i = 0 ; i<PLAZAS; i++){
                if(aparcamiento[i] == 0 && aparcamiento [i + 1] == 0){
                    //pthread_mutex_lock(&mutex[i]);
                    break;
                }
            }

            aparcamiento[i] = coche_id;
            aparcamiento[i+1] = coche_id;
            estado_coches[coche_id] = 1;
            plazas_libres -= 2;
            //pthread_mutex_unlock(&mutex[i]);
            printf("ENTRADA: Camión %d aparca en %d. Plazas libres: %d.\n", coche_id, i, plazas_libres);
            print_estado();
            pthread_mutex_unlock(&mutexGen);

            sleep((rand() % 5 ) + 1);

            pthread_mutex_lock(&mutexGen);
            //pthread_mutex_lock(&mutex[i]);
            aparcamiento[i] = 0;
            aparcamiento[i+1] = 0;
            estado_coches[coche_id] = 0;
            plazas_libres += 2;
            printf("SALIENDO: Camión %d saliendo. Plazas libres: %d.\n", coche_id, plazas_libres);
            print_estado();
            //pthread_mutex_unlock(&mutex[i]);
        }
        pthread_mutex_unlock(&mutexGen);
        sleep((rand() % 10 ) + 1);
    }
}

void * cocheAparca(void *num){
    int coche_id = *(int *) num;
    int ocupado = 1;
    int i,j;
    //sleep(2);
    while(1){
        estado_coches[coche_id] = 0;
        pthread_mutex_lock(&mutexGen);
        if(plazas_libres > 0){
            for(i = 0 ; i<PLANTAS; i++){
                for(j = 0; j<PLAZAS; j++){
                    if(aparcamiento_plantas[i][j] == 0){
                        break;
                    }
                }
                if(aparcamiento_plantas[i][j] == 0){
                    break;
                }
            }

            aparcamiento_plantas[i][j] = coche_id;
            estado_coches[coche_id] = 1;
            plazas_libres -= 1;
            //pthread_mutex_unlock(&mutex[i]);
            printf("ENTRADA: coche %d aparca en %d de la planta %d. Plazas libres: %d.\n", coche_id, j, i, plazas_libres);
            print_estado();
            pthread_mutex_unlock(&mutexGen);

            sleep((rand() % 5 ) + 1);

            pthread_mutex_lock(&mutexGen);
            //pthread_mutex_lock(&mutex[i]);
            aparcamiento_plantas[i][j] = 0;
            estado_coches[coche_id] = 0;
            plazas_libres += 1;
            printf("SALIENDO: Coche %d saliendo. Plazas libres: %d.\n", coche_id, plazas_libres);
            print_estado();
            //pthread_mutex_unlock(&mutex[i]);
        }
        pthread_mutex_unlock(&mutexGen);
        sleep((rand() % 5 ) + 1);
    }
}

int main(){
    pthread_t coches[COCHES];
    pthread_t camiones[CAMIONES];
    int i, j;
    int coche_id[COCHES + CAMIONES];
    pthread_mutex_init(&mutexGen, NULL);
    for(i = 0; i<PLANTAS; i++){
        for(j = 0; j<PLAZAS; j++){
            aparcamiento_plantas[i][j] = 0;
            pthread_mutex_init(&mutex[i], NULL);
        }

    }


    for(i = 0; i<COCHES; i++){
        //pthread_cond_init(&espera,  NULL);
        coche_id[i] = i+1;
        pthread_create(&coches[i], NULL, cocheAparca, (void *) &coche_id[i]);
    }

/*
    for(j = i; j-i<CAMIONES; j++){
        //pthread_cond_init(&espera,  NULL);
        coche_id[j] = j+1 + 100;
        pthread_create(&camiones[j], NULL, camionAparca, (void *) &coche_id[j]);
    }
*/

    while(1);
    //comentario

}
