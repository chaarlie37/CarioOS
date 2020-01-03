#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


int COCHES;
int CAMIONES;
int PLAZAS;
int PLANTAS;

int ** aparcamiento;

int plazas_libres;
pthread_mutex_t mutex;
pthread_cond_t * espera_coches;
pthread_cond_t * espera_camiones;

int * coche_id;
int * camion_id;

int * contador;
int offset_camiones;

// funcion para imprimir el estado del parking
void print_estado(){
    int i,j;
    printf("Parking: \n");
    for(i = 0; i<PLANTAS; i++){
        for(j = 0; j<PLAZAS; j++){
            printf("[%d]", aparcamiento[i][j]);
        }
        printf("\n");
    }
    // (opcional para testear la inanicion)
    // imprime el array "contadores" que almacena el número de veces que aparca cada vehículo
    /*
    printf("CONTADORES: \n");
    for(i = 0; i<COCHES; i++){
        printf("[%d: %d]", coche_id[i], contador[i]);
    }
    for(i = 0; i<CAMIONES; i++){
        printf("[%d: %d]", camion_id[i] + offset_camiones, contador[i + COCHES]);
    }
    */
    printf("\n\n");
}

void *camionAparca(void *num){
    int camion_id = *(int *) num;   // identificador del camión que se mostrará en el parking
    int i,j,o,k,libres;
    while(1){
        // se bloquea el mutex
        pthread_mutex_lock(&mutex);
        // se verifica que hay plazas libres para un camión (no vale sólo verificar que plazas_libres > 1 porque un Camión
        // necesita 2 contiguas y en la misma planta)
        libres = 0;
        if(plazas_libres > 1){
            for(o = 0; o<PLANTAS; o++){
                for(k = 0; k<PLAZAS; k++){
                    if(aparcamiento[o][k-1] == 0 && aparcamiento[o][k] == 0){
                        libres = 1;
                    }
                }
                if(k<PLAZAS && aparcamiento[i][j-1] == 0 && aparcamiento[o][k] == 0){
                    libres = 1;
                }
            }
        }

        // mientras que no haya plazas libres, espera
        while(!libres){
            pthread_cond_wait(&espera_camiones[camion_id - 1], &mutex);
            libres = 1;
        }

        if(plazas_libres > 1){
            // se recorre el parking hasta encontrar las 2 primeras plazas libres para aparcar
            for(i = 0 ; i<PLANTAS; i++){
                for(j = 1; j<PLAZAS; j++){
                    if(aparcamiento[i][j-1] == 0 && aparcamiento[i][j] == 0){
                        break;
                    }
                }
                if(j<PLAZAS && aparcamiento[i][j-1] == 0 && aparcamiento[i][j] == 0){
                    break;
                }
            }

            // una vez encontradas las plazas, proceso de aparcar y de salir del parking
            if(j<PLAZAS && aparcamiento[i][j-1] == 0 && aparcamiento[i][j] == 0){
                // se aparca
                aparcamiento[i][j] = camion_id + offset_camiones;
                aparcamiento[i][j-1] = camion_id + offset_camiones;
                plazas_libres -= 2;
                // contador para verificar la inanicion
                contador[camion_id - 1 + COCHES]++;
                // se imprime la accion de aparcar y el estado del parking
                printf("ENTRADA: Camión %d aparca en %d y %d de planta %d. Plazas libres: %d.\n", camion_id + offset_camiones, j-1, j, i, plazas_libres);
                print_estado();
                // se libera el mutex una vez aparcado y se espera un tiempo aleatorio
                pthread_mutex_unlock(&mutex);
                sleep((rand()%5)+1 );
                // una vez transcurrido el tiempo aleatorio, se bloquea el mutex para salir del parking
                pthread_mutex_lock(&mutex);
                // se dejan las plazas con el valor 0 = libre
                aparcamiento[i][j] = 0;
                aparcamiento[i][j-1] = 0;
                plazas_libres += 2;
                // se indica por pantalla y se muestra el estado nuevo del parking
                printf("SALIENDO: Camión %d saliendo. Plazas libres: %d.\n", camion_id + offset_camiones, plazas_libres);
                print_estado();
            }
        }

        // una vez el camión sale del parking indica al resto de vehículos que estén esperando que hay plazas disponibles
        for(i = 0; i< COCHES; i++){
            pthread_cond_signal(&espera_coches[i]);
        }
        for(i = 0; i<CAMIONES; i++){
            pthread_cond_signal(&espera_camiones[i]);
        }
        // se libera el mutex y el camión espera un tiempo aleatorio fuera del parking para volver a entrar
        pthread_mutex_unlock(&mutex);
        sleep((rand() % 10 ) + 1);
    }
}

void * cocheAparca(void *num){
    // identificador del camión que se mostrará en el parking
    int coche_id = *(int *) num;
    int i,j,o,k,libres;
    while(1){
        // se bloquea el mutex
        pthread_mutex_lock(&mutex);
        // mientras que no haya plazas libres, espera
        while(plazas_libres == 0){
            pthread_cond_wait(&espera_coches[coche_id - 1], &mutex);
        }
        if(plazas_libres > 0){
            // se recorre el parking hasta encontrar la primera plaza libre para aparcar
            for(i = 0 ; i<PLANTAS; i++){
                for(j = 0; j<PLAZAS; j++){
                    if(aparcamiento[i][j] == 0){
                        break;
                    }
                }
                if(j < PLAZAS && aparcamiento[i][j] == 0){
                    break;
                }
            }
            // una vez encontradas las plazas, aparca
            aparcamiento[i][j] = coche_id;
            plazas_libres--;
            // contador para verificar la inanicion
            contador[coche_id - 1]++;
            // se imprime la accion de aparcar y el estado del parking
            printf("ENTRADA: coche %d aparca en %d de la planta %d. Plazas libres: %d.\n", coche_id, j, i, plazas_libres);
            print_estado();
            // se libera el mutex una vez aparcado y se espera un tiempo aleatorio
            pthread_mutex_unlock(&mutex);
            sleep((rand() % 5 ) + 1);
            // una vez transcurrido el tiempo aleatorio, se bloquea el mutex para salir del parking
            pthread_mutex_lock(&mutex);
            // se dejan las plazas con el valor 0 = libre
            aparcamiento[i][j] = 0;
            plazas_libres++;
            // se indica por pantalla y se muestra el estado nuevo del parking
            printf("SALIENDO: Coche %d saliendo. Plazas libres: %d.\n", coche_id, plazas_libres);
            print_estado();

            // una vez el camión sale del parking indica al resto de coches que estén esperando que hay plazas disponibles
            // (no se indica a los camiones ya que al salir un coche deja sólo una plaza libre)
            for(i = 0; i<COCHES; i++){
                pthread_cond_signal(&espera_coches[i]);
            }

            // se comprueba si hay plazas libres para un camión y avisa a los que estén esperando
            for(o = 0; o<PLANTAS; o++){
                for(k = 1; k<PLAZAS; k++){
                    if(aparcamiento[o][k-1] == 0 && aparcamiento[o][k] == 0){
                        libres = 1;
                        break;
                    }
                }
                if(k<PLAZAS && aparcamiento[o][k-1] == 0 && aparcamiento[o][k] == 0){
                    libres = 1;
                    break;
                }
            }
            if(libres){
                for(i = 0; i<CAMIONES; i++){
                    pthread_cond_signal(&espera_camiones[i]);
                }
            }
        }
        // se libera el mutex y el camión espera un tiempo aleatorio fuera del parking para volver a entrar
        pthread_mutex_unlock(&mutex);
        sleep((rand() % 10 ) + 1);
    }
}


int main(int argc, char *argv[]){

    int i,j;
    int digitos;
    int aux;

    // array de threads de coches y de camiones (cada thread es un vehículo)
    pthread_t * coches;
    pthread_t * camiones;

    switch (argc){
        case 3:
                CAMIONES = 0;
                PLAZAS = atoi(argv[1]);
                PLANTAS = atoi(argv[2]);
                COCHES = PLANTAS * PLAZAS * 2;
        break;
        case 4:
                CAMIONES = 0;
                COCHES = atoi(argv[3]);
                PLAZAS = atoi(argv[1]);
                PLANTAS = atoi(argv[2]);
        break;
        case 5:
                PLAZAS = atoi(argv[1]);
                PLANTAS = atoi(argv[2]);
                COCHES = atoi(argv[3]);
                CAMIONES = atoi(argv[4]);
        break;
        default:
                printf("Error. Introduce como argumentos PLAZAS PLANTAS, o PLAZAS PLANTAS COCHES, o PLAZAS PLANTAS COCHES CAMIONES.\n");
                exit(1);
        break;
    }

    if(PLAZAS == 0 || PLANTAS == 0){
        printf("Error. El valor PLAZAS y PLANTAS debe ser mayor que 0.\n");
        exit(2);
    }

    plazas_libres = PLAZAS * PLANTAS;
    aparcamiento = (int **) malloc(PLANTAS * sizeof(int *));
    for(i = 0; i<PLANTAS; i++){
        aparcamiento[i] = (int *) malloc(PLAZAS * sizeof(int));
    }
    espera_coches = malloc(COCHES * sizeof(pthread_cond_t));
    espera_camiones = malloc(CAMIONES * sizeof(pthread_cond_t));
    contador = malloc((COCHES +  CAMIONES) * sizeof(int));
    coche_id = malloc(COCHES * sizeof(int));
    camion_id = malloc(CAMIONES * sizeof(int));
    coches = malloc(COCHES * sizeof(pthread_t));
    camiones = malloc(CAMIONES * sizeof(pthread_t));

    // cálculo de cuántos dígitos tiene el número de coches para que los ID de los camiones tenga un dígito más por lo menos
    // para diferenciar los IDs de coches y de camiones
    // es para cuando hay +100 coches, los IDs de los camiones sean a partir del 1001

    digitos = 1;
    aux = COCHES;
    while(aux / 10 >= 1){
        digitos++;
        aux = aux / 10;
    }
    offset_camiones = 1;
    for(i = 0; i<digitos; i++){
        offset_camiones = offset_camiones * 10;
    }


    // se inicializa el mutex
    pthread_mutex_init(&mutex, NULL);
    // se inicializa la matriz que simula el parking
    for(i = 0; i<PLANTAS; i++){
        for(j = 0; j<PLAZAS; j++){
            aparcamiento[i][j] = 0;
        }
    }

    // se inicializan los coches
    for(i = 0; i<COCHES; i++){
        pthread_cond_init(&espera_coches[i],  NULL);
        coche_id[i] = i + 1;
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
