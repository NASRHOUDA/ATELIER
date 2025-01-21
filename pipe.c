#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>

#define TOKEN "TOKEN"
#define BUFFER_SIZE 256
#define NUM_STATIONS 6

bool station_active[NUM_STATIONS];

void simuler_defaillance(int station_id) {
    station_active[station_id] = 0;
    printf("Station %d : Defaillance...\n", station_id);
}

int main() {
    int pipes[NUM_STATIONS][2];
    pid_t pids[NUM_STATIONS];
    char buffer[BUFFER_SIZE];
    int i;

    for (i = 0; i < NUM_STATIONS; i++) {
        station_active[i] = 1;
    }

    for (i = 0; i < NUM_STATIONS; i++) {
        if (pipe(pipes[i]) != 0) {
            printf("Erreur: pb creation pipe\n");
            return 1;
        }
    }

    for (i = 0; i < NUM_STATIONS; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            printf("Erreur: pb fork\n");
            return 1;
        }

        if (pids[i] == 0) {
            int station_id = i;
            close(pipes[station_id][1]);
            close(pipes[(station_id + 1) % NUM_STATIONS][0]);

            while (1) {
                if (read(pipes[station_id][0], buffer, BUFFER_SIZE) <= 0) {
                    continue;
                }

                if (strcmp(buffer, TOKEN) == 0) {
                    printf("Station %d : Jeton recu.\n", station_id);

                    if (station_id == 1) {
                        printf("Station %d : J'envoie un message..\n", station_id);
                        write(pipes[(station_id + 1) % NUM_STATIONS][1], "Message St1", strlen("Message St1") + 1);
                    } else {
                        printf("Station %d : Rien a envoyer. Passer jeton.\n", station_id);
                        int next_station = (station_id + 1) % NUM_STATIONS;
                        while (station_active[next_station] == 0) {
                            next_station = (next_station + 1) % NUM_STATIONS;
                        }
                        write(pipes[next_station][1], TOKEN, strlen(TOKEN) + 1);
                    }
                } else {
                    printf("Station %d : Recu msg - %s\n", station_id, buffer);
                    int next_station = (station_id + 1) % NUM_STATIONS;
                    while (station_active[next_station] == 0) {
                        next_station = (next_station + 1) % NUM_STATIONS;
                    }
                    write(pipes[next_station][1], TOKEN, strlen(TOKEN) + 1);
                }
            }
            exit(0);
        }
    }

    for (i = 0; i < NUM_STATIONS; i++) {
        close(pipes[i][0]);
    }

    sleep(4);
    simuler_defaillance(3);

    write(pipes[0][1], TOKEN, strlen(TOKEN) + 1);
    printf("Main : Jeton envoye a la st0.\n");

    for (i = 0; i < NUM_STATIONS; i++) {
        wait(NULL);
    }

    return 0;
}
