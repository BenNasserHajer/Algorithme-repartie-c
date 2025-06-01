#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "message_data.h"

#define PROCESS_ID 1
#define PROCESS_PORT 5002

int horloge[NB_PROCESSUS][NB_PROCESSUS] = {0};
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int compteur_evenements = 0;  // AJOUTÉ

void afficher_horloge(const char *prefix) {
    printf("[%s] Horloge Processus %d:\n", prefix, PROCESS_ID + 1);
    for (int i = 0; i < NB_PROCESSUS; i++) {
        printf("[ ");
        for (int j = 0; j < NB_PROCESSUS; j++) {
            printf("%d ", horloge[i][j]);
        }
        printf("]\n");
    }
    printf("\n");

}

int attendre_demarrage() {
    char buffer[64];
    int sock;
    struct sockaddr_in coord = {0};

    while (1) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        coord.sin_family = AF_INET;
        coord.sin_port = htons(5000);
        coord.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sock, (struct sockaddr *)&coord, sizeof(coord)) == 0) {
            sprintf(buffer, "REGISTER:%d", PROCESS_ID);
            write(sock, buffer, strlen(buffer));
            read(sock, buffer, sizeof(buffer));
            close(sock);
            if (strncmp(buffer, "START", 5) == 0) return 1;
            sleep(1);
        } else {
            sleep(1);
        }
    }
}

void evenement_local(const char *desc) {
    pthread_mutex_lock(&lock);
    horloge[PROCESS_ID][PROCESS_ID]++;
    compteur_evenements++;  // AJOUTÉ
    printf("[P2] Événement local #%d: %s\n", compteur_evenements, desc);  // LIGNE MODIFIÉE
    afficher_horloge("Local");
    pthread_mutex_unlock(&lock);
}

void envoyer_message(const char *msg, int port, int dest) {
    pthread_mutex_lock(&lock);

    horloge[PROCESS_ID][PROCESS_ID]++;
    horloge[PROCESS_ID][dest]++;

    MessageData data;
    strncpy(data.message, msg, MAX_MESSAGE_LEN);
    for (int i = 0; i < NB_PROCESSUS; i++)
        for (int j = 0; j < NB_PROCESSUS; j++)
            data.horloge[i][j] = horloge[i][j];

    pthread_mutex_unlock(&lock);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in dest_addr = {0};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) == 0) {
        write(sock, &data, sizeof(data));
        printf("[P2] Envoi: %s vers port %d\n", msg, port);
        afficher_horloge("Après envoi");
    } else {
        perror("Erreur d'envoi");
    }

    close(sock);
}

void *reception_thread(void *arg) {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PROCESS_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_sock, 5);

    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        MessageData data;
        read(client_sock, &data, sizeof(data));

        pthread_mutex_lock(&lock);
        horloge[PROCESS_ID][PROCESS_ID]++;
        for (int i = 0; i < NB_PROCESSUS; i++) {
            for (int j = 0; j < NB_PROCESSUS; j++) {
                if (data.horloge[i][j] > horloge[i][j])
                    horloge[i][j] = data.horloge[i][j];
            }
        }

        printf("[P2] Réception: %s\n", data.message);
        afficher_horloge("Après réception");
        pthread_mutex_unlock(&lock);
        close(client_sock);
    }

    return NULL;
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, reception_thread, NULL);

    if (!attendre_demarrage()) {
        printf("Erreur de démarrage\n");
        return 1;
    }

    sleep(1);
    evenement_local("Local");
    sleep(10);
    envoyer_message("P2 -> P1 : ", 5001, 0);
    sleep(10);
    evenement_local("Local");
    sleep(10);
    envoyer_message("P2 -> P3 :  ", 5003, 2);
    sleep(10);
    evenement_local("Local");
    sleep(10);
    envoyer_message("P2 -> P4 : ", 5004, 3);
    sleep(10);
    evenement_local("Local");
    sleep(10);
    printf("Processus 2 terminé\n");

    return 0;
}
