#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>  
#define TOKEN "TOKEN"   
#define BUFFER_SIZE 256  
#define NUM_STATIONS 6   
int main() {
    int pipes[NUM_STATIONS][2];  
    pid_t pids[NUM_STATIONS];   
    char buffer[BUFFER_SIZE];   
    for (int i = 0; i < NUM_STATIONS; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Erreur lors de la création des pipes");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < NUM_STATIONS; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("Erreur lors du fork");
            exit(EXIT_FAILURE);
        }
        if (pids[i] == 0) {  
            int station_id = i;  
            close(pipes[station_id][1]);  
            close(pipes[(station_id + 1) % NUM_STATIONS][0]);  
            while (true) {
                read(pipes[station_id][0], buffer, BUFFER_SIZE);
                if (strcmp(buffer, TOKEN) == 0) {
                    printf("Station %d : Jeton reçu.\n", station_id);
                    if (station_id == 1) {  
                        printf("Station %d : Transmission d'une trame.\n", station_id);
                       
                        write(pipes[(station_id + 1) % NUM_STATIONS][1], "Message de Station 1", strlen("Message de Station 1") + 1);
                    } else {
                        printf("Station %d : Aucun message. Jeton passé.\n", station_id);
                        write(pipes[(station_id + 1) % NUM_STATIONS][1], TOKEN, strlen(TOKEN) + 1);  
                    }
                } else {  
                    printf("Station %d : Message reçu - \"%s\"\n", station_id, buffer);
                    write(pipes[(station_id + 1) % NUM_STATIONS][1], TOKEN, strlen(TOKEN) + 1);
                }
            }
            exit(EXIT_SUCCESS);  
        }
    }
    for (int i = 0; i < NUM_STATIONS; i++) {
        close(pipes[i][0]);  
    }
    write(pipes[0][1], TOKEN, strlen(TOKEN) + 1);
    printf("Processus principal : Jeton initial envoyé à la station 0.\n");
    for (int i = 0; i < NUM_STATIONS; i++) {
        wait(NULL);
    }
    return 0;
}
