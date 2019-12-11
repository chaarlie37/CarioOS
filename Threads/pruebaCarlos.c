#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


#define COCHES 30
#define CAMIONES 5
#define PLAZAS 8
#define PLANTAS 3
int aparcamiento_plantas[PLANTAS][PLAZAS];
int aparcamiento[PLAZAS];

int plazas_libres = PLAZAS * PLANTAS;
pthread_mutex_t mutexGen;
pthread_mutex_t mutex[PLAZAS];
pthread_cond_t espera_coches[COCHES];
pthread_cond_t espera_camiones[CAMIONES];

int contador[COCHES];

int coche_id[COCHES];
int camion_id[CAMIONES];

void print_estado(){
    printf("Parking: \n");
    for(int i = 0; i<PLANTAS; i++){
        for(int j = 0; j<PLAZAS; j++){
            printf("[%d]", aparcamiento_plantas[i][j]);
        }
        printf("\n");
    }
    printf("CONTADORES: \n");
    for(int i = 0; i<COCHES; i++){
        printf("[%d: %d]", coche_id[i], contador[i]);
    }
    for(int i = 0; i<CAMIONES; i++){
        printf("[%d: %d]", camion_id[i] + 100, contador[i + COCHES]);
    }
    printf("\n\n");
}

void *camionAparca(void *num){
    int camion_id = *(int *) num;   // identificador del camión que se mostrará en el parking
    int i,j;
    while(1){
        // se bloquea el mutex
        pthread_mutex_lock(&mutexGen);
        // se verifica que hay plazas libres para un camión (no vale sólo verificar que plazas_libres > 1 porque un Camión
        // necesita 2 contiguas y en la misma planta)
        int libres = 0;
        if(plazas_libres > 1){
            int o = 0;
            int k = 0;
            for(o = 0; o<PLANTAS; o++){
                for(k = 0; k<PLAZAS; k++){
                    if(aparcamiento_plantas[o][k-1] == 0 && aparcamiento_plantas[o][k] == 0){
                        libres = 1;
                    }
                }
                if(k<PLAZAS && aparcamiento_plantas[i][j-1] == 0 && aparcamiento_plantas[o][k] == 0){
                    libres = 1;
                }
            }
        }

        // mientras que no haya plazas libres, espera
        while(!libres){
            pthread_cond_wait(&espera_camiones[camion_id - 1], &mutexGen);
            libres = 1;
            /*
            for(o = 0; o<PLANTAS; o++){
                for(k = 0; k<PLAZAS; k++){
                    if(aparcamiento_plantas[o][k-1] == 0 && aparcamiento_plantas[o][k] == 0){
                        libres = 1;
                    }
                }
                if(k<PLAZAS && aparcamiento_plantas[i][j-1] == 0 && aparcamiento_plantas[o][k] == 0){
                    libres = 1;
                }
            }
            */
        }

        if(plazas_libres > 1){

            // se recorre el parking hasta encontrar las 2 primeras plazas libres para aparcar
            for(i = 0 ; i<PLANTAS; i++){
                for(j = 1; j<PLAZAS; j++){
                    if(aparcamiento_plantas[i][j-1] == 0 && aparcamiento_plantas[i][j] == 0){
                        break;
                    }
                }
                if(j<PLAZAS && aparcamiento_plantas[i][j-1] == 0 && aparcamiento_plantas[i][j] == 0){
                    break;
                }
            }

            // una vez encontradas las plazas, proceso de aparcar y de salir del parking
            if(j<PLAZAS && aparcamiento_plantas[i][j-1] == 0 && aparcamiento_plantas[i][j] == 0){
                // se aparca
                aparcamiento_plantas[i][j] = camion_id + 100;
                aparcamiento_plantas[i][j-1] = camion_id + 100;
                plazas_libres -= 2;
                // contador para verificar la inanicion
                contador[camion_id - 1 + COCHES]++;
                // se imprime la accion de aparcar y el estado del parking
                printf("ENTRADA: Camión %d aparca en %d y %d de planta %d. Plazas libres: %d.\n", camion_id + 100, j-1, j, i, plazas_libres);
                print_estado();
                // se libera el mutex una vez aparcado y se espera un tiempo aleatorio
                pthread_mutex_unlock(&mutexGen);
                sleep((rand()%10)+1 );
                // una vez transcurrido el tiempo aleatorio, se bloquea el mutex para salir del parking
                pthread_mutex_lock(&mutexGen);
                // se dejan las plazas con el valor 0 = libre
                aparcamiento_plantas[i][j] = 0;
                aparcamiento_plantas[i][j-1] = 0;
                plazas_libres += 2;
                // se indica por pantalla y se muestra el estado nuevo del parking
                printf("SALIENDO: Camión %d saliendo. Plazas libres: %d.\n", camion_id + 100, plazas_libres);
                print_estado();
            }
        }

        // una vez el camión sale del parking indica al resto de vehículos que estén esperando que hay plazas disponibles
        for(int i = 0; i< COCHES; i++){
            pthread_cond_signal(&espera_coches[i]);
        }
        for(int i = 0; i<CAMIONES; i++){
            pthread_cond_signal(&espera_camiones[i]);
        }
        // se libera el mutex y el camión espera un tiempo aleatorio fuera del parking para volver a entrar
        pthread_mutex_unlock(&mutexGen);
        sleep((rand() % 10 ) + 1);
    }
}

void * cocheAparca(void *num){
    // identificador del camión que se mostrará en el parking
    int coche_id = *(int *) num;
    int i,j;
    while(1){
        // se bloquea el mutex
        pthread_mutex_lock(&mutexGen);
        // mientras que no haya plazas libres, espera
        while(plazas_libres == 0){
            pthread_cond_wait(&espera_coches[coche_id - 1], &mutexGen);
        }
        if(plazas_libres > 0){
            // se recorre el parking hasta encontrar la primera plaza libre para aparcar
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
            // una vez encontradas las plazas, aparca
            aparcamiento_plantas[i][j] = coche_id;
            plazas_libres -= 1;
            // contador para verificar la inanicion
            contador[coche_id - 1]++;
            // se imprime la accion de aparcar y el estado del parking
            printf("ENTRADA: coche %d aparca en %d de la planta %d. Plazas libres: %d.\n", coche_id, j, i, plazas_libres);
            print_estado();
            // se libera el mutex una vez aparcado y se espera un tiempo aleatorio
            pthread_mutex_unlock(&mutexGen);
            sleep((rand() % 10 ) + 1);
            // una vez transcurrido el tiempo aleatorio, se bloquea el mutex para salir del parking
            pthread_mutex_lock(&mutexGen);
            // se dejan las plazas con el valor 0 = libre
            aparcamiento_plantas[i][j] = 0;
            plazas_libres += 1;
            // se indica por pantalla y se muestra el estado nuevo del parking
            printf("SALIENDO: Coche %d saliendo. Plazas libres: %d.\n", coche_id, plazas_libres);
            print_estado();

            // una vez el camión sale del parking indica al resto de coches que estén esperando que hay plazas disponibles
            // (no se indica a los camiones ya que al salir un coche deja sólo una plaza libre)
            for(int i = 0; i<COCHES; i++){
                pthread_cond_signal(&espera_coches[i]);
            }

            // se comprueba si hay plazas libres para un camión y avisa a los que estén esperando
            int libres = 0;
            int o = 0;
            int k = 0;
            for(o = 0; o<PLANTAS; o++){
                for(k = 1; k<PLAZAS; k++){
                    if(aparcamiento_plantas[o][k-1] == 0 && aparcamiento_plantas[o][k] == 0){
                        libres = 1;
                        break;
                    }
                }
                if(k<PLAZAS && aparcamiento_plantas[o][k-1] == 0 && aparcamiento_plantas[o][k] == 0){
                    libres = 1;
                    break;
                }
            }
            if(libres){
                for(int i = 0; i<CAMIONES; i++){
                    pthread_cond_signal(&espera_camiones[i]);
                }
            }
        }
        // se libera el mutex y el camión espera un tiempo aleatorio fuera del parking para volver a entrar
        pthread_mutex_unlock(&mutexGen);
        sleep((rand() % 5 ) + 1);
    }
}

int main(){
    
    // array de threads de coches y de camiones (cada thread es un vehículo)
    pthread_t coches[COCHES];
    pthread_t camiones[CAMIONES];
    int i, j;
    // se inicializa el mutex
    pthread_mutex_init(&mutexGen, NULL);
    // se inicializa la matriz que simula el parking
    for(i = 0; i<PLANTAS; i++){
        for(j = 0; j<PLAZAS; j++){
            aparcamiento_plantas[i][j] = 0;
        }
    }

    // se inicializan los coches
    for(i = 0; i<COCHES; i++){
        pthread_cond_init(&espera_coches[i],  NULL);
        coche_id[i] = i+1;
        pthread_create(&coches[i], NULL, cocheAparca, (void *) &coche_id[i]);
    }

    // se inicializan los camiones
    for(j = 0; j<CAMIONES; j++){
        pthread_cond_init(&espera_camiones[j],  NULL);
        camion_id[j] = j + 1;
        pthread_create(&camiones[j], NULL, camionAparca, (void *) &camion_id[j]);
    }

    while(1);

}
