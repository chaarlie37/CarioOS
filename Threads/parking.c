#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


#define COCHES 10
#define PLAZAS COCHES/2
int estado_coches[COCHES];
int aparcamiento[PLAZAS];
int plazas_libres = PLAZAS;
pthread_mutex_t mutexGen;
pthread_mutex_t mutex[PLAZAS];
pthread_cond_t espera[COCHES];

void print_estado(){
    printf("Parking: ");
    for(int i= 0; i<PLAZAS; i++){
        printf("[%d]", aparcamiento[i]);
    }
    printf("\n");
}

void *funcion(void *num){
    int coche_id = *(int *) num;
    int ocupado = 1;
    int i;
    //sleep(2);
    while(1){
        estado_coches[coche_id] = 0;
        pthread_mutex_lock(&mutexGen);
        if(plazas_libres > 0){

            for(i =0 ; i<PLAZAS; i++){
                if(aparcamiento[i] == 0){
                    //pthread_mutex_lock(&mutex[i]);
                    break;
                }
            }

            aparcamiento[i] = coche_id;
            estado_coches[coche_id] = 1;
            plazas_libres -= 1;
            //pthread_mutex_unlock(&mutex[i]);
            printf("ENTRADA: coche %d aparca en %d. Plazas libres: %d.\n", coche_id, i, plazas_libres);
            print_estado();
            pthread_mutex_unlock(&mutexGen);

            sleep((rand() % 5 ) + 1);

            pthread_mutex_lock(&mutexGen);
            //pthread_mutex_lock(&mutex[i]);
            aparcamiento[i] = 0;
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
    pthread_t th[COCHES];
    int i;
    int coche_id[COCHES];
    pthread_mutex_init(&mutexGen, NULL);
    for(i = 0; i<PLAZAS; i++){
        aparcamiento[i] = 0;
        pthread_mutex_init(&mutex[i], NULL);
    }

    for(i = 0; i<COCHES; i++){
        //pthread_cond_init(&espera,  NULL);
        coche_id[i] = i+1;
        pthread_create(&th[i], NULL, funcion, (void *) &coche_id[i]);
    }

    while(1);
    //comentario

}
