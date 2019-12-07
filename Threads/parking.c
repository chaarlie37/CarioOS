#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


#define COCHES 10
int estado_coches[COCHES];
int aparcamiento[COCHES/2];
int plazas_libres = COCHES/2;
pthread_mutex_t mutexGen;
pthread_mutex_t mutex[COCHES/2];
pthread_cond_t espera[COCHES];

void print_estado(){
    printf("Parking: ");
    for(int i= 0; i<COCHES/2; i++){
        printf("[%d]", aparcamiento[i]);
    }
    printf("\n");
}

void *funcion(void *num){
    int coche_id = *(int *) num;
    int ocupado = 1;
    int i;
    sleep(2);
    while(1){
        estado_coches[coche_id] = 0;
        
        if(plazas_libres > 0){
            for(i =0 ; i<COCHES/2; i++){
                if(aparcamiento[i] == 0){
                    pthread_mutex_lock(&mutex[i]);
                    break;
                }
            }
        }
        pthread_mutex_lock(&mutexGen);
        aparcamiento[i] = coche_id;
        estado_coches[coche_id] = 1;
        plazas_libres -= 1;
        pthread_mutex_unlock(&mutex[i]);

        printf("ENTRADA: coche %d aparca en %d. Plazas libres: %d.\n", coche_id, i, plazas_libres);
        print_estado();
        sleep((rand()%2)+1);

        pthread_mutex_lock(&mutex[i]);
        aparcamiento[i] = 0;
        estado_coches[coche_id] = 0;
        plazas_libres += 1;
        printf("SALIENDO: Coche %d saliendo. Plazas libres: %d.\n", coche_id, plazas_libres);
        print_estado();
        pthread_mutex_unlock(&mutex[i]);
        pthread_mutex_unlock(&mutexGen);





    }
}

int main(){
    pthread_t th;
    int i;
    int coche_id[COCHES];
    pthread_mutex_init(&mutexGen, NULL);
    for(i = 0; i<COCHES/2; i++){
        aparcamiento[i] = 0;
        pthread_mutex_init(&mutex[i], NULL);
    }

    for(i = 0; i<COCHES; i++){
        //pthread_cond_init(&espera,  NULL);
        coche_id[i] = i+1;
        pthread_create(&th, NULL, funcion, (void *) &coche_id[i]);
    }

    while(1);
    //comentario

}

