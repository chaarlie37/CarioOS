#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define COCHES 10
int estado_coches[COCHES];
int aparcamiento[COCHES/2];
int plazas_libres = COCHES/2;
pthread_mutex_t mutex[COCHES];
pthread_cond_t espera[COCHES];

//char *estado(int i){
  //  if(aparcamiento[i] != 0){

  //  }
//}
//Funcion para imprimir la lista de aparcamientos y ver cuales están ocupados
void print_estado(){
    printf("Parking: ");
    for(int i= 0; i<COCHES; i++){
        printf("[%d]", aparcamiento[i]);
    }
    printf("\n");
}

void *coche(void *num){
    int id = *(int *) num;
}


int main(){
    int i;
    pthread_t th;
    int id_coches[COCHES];

    for(i = 0; i<COCHES; i++){
        
        pthread_mutex_init(&mutex[i], NULL);
        aparcamiento[i] = 0;
        pthread_cond_init(&espera, NULL);
        id_coches[i] = i;
        pthread_create(&th, NULL, coche, (void *) &id_coches[i]);
        
    
    }

    while(1);

}
