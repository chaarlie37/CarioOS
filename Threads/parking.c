#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define COCHES 10
int estado_coches[COCHES];
int aparcamiento[COCHES/2];
int plazas_libres = COCHES/2;
pthread_mutex_t mutex;
pthread_cond_t espera[COCHES];

//char *estado(int i){
  //  if(aparcamiento[i] != 0){

  //  }
//}

void print_estado(){
    printf("Parking: ");
    for(int i= 0; i<COCHES; i++){
        printf("[%d]", aparcamiento[i]);
    }
    printf("\n");
}

void *funcion(void *num){
    int coche_id = *(int *) num;

    while(1){

    }
}

int main(){
    pthread_t th;
    int i;
    int coche_id[COCHES];
    pthread_mutex_init(&mutex, NULL);
    for(i = 0; i<COCHES; i++){
        aparcamient[i] = 0;
        pthread_cond_init(&espera,  NULL);
        coche_id[i] = i;
        pthread_create(&th, NULL, funcion, (void *) &coche_id[i]);
    }

    while(1);

}
