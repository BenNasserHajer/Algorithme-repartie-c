#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "message_data_vect.h"

#define PROCESS_ID 0
#define PROCESS_PORT 5001

int horloge_vectorielle[NB_PROCESSUS] = {0};
int compteur_evenements = 0;
pthread_mutex_t horloge_mutex = PTHREAD_MUTEX_INITIALIZER;

void afficher_horloge() {
    printf("Horloge vectorielle : [ ");
    for (int i = 0; i < NB_PROCESSUS; i++) {
        printf("%d ", horloge_vectorielle[i]);
    }
    printf("]\n");
    printf("\n");
}

int attendre_demarrage() {
    int sock;
    char buffer[64];
    struct sockaddr_in coord;

    while (1) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        coord.sin_family = AF_INET;
        coord.sin_port = htons(5000);
        coord.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sock, (struct sockaddr *)&coord, sizeof(coord)) == 0) {
            sprintf(buffer, "REGISTER:%d\n", PROCESS_ID);
            write(sock, buffer, strlen(buffer));
            read(sock, buffer, sizeof(buffer));
            close(sock);

            if (strncmp(buffer, "START", 5) == 0) return 1;
        }

        sleep(1);
    }
}

void evenement_local(const char *desc) {
    pthread_mutex_lock(&horloge_mutex);
    horloge_vectorielle[PROCESS_ID]++;
    compteur_evenements++;
    printf("[P1] Événement local #%d: %s\n", compteur_evenements, desc);
    afficher_horloge();
    pthread_mutex_unlock(&horloge_mutex);
}

void envoyer_message(const char *msg, int port) {
    pthread_mutex_lock(&horloge_mutex);
    horloge_vectorielle[PROCESS_ID]++;

    MessageDataVect data;
    strncpy(data.message, msg, MAX_MESSAGE_LEN);
    for (int i = 0; i < NB_PROCESSUS; i++) {
        data.horloge[i] = horloge_vectorielle[i];
    }
    pthread_mutex_unlock(&horloge_mutex);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    dest.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&dest, sizeof(dest)) == 0) {
        write(sock, &data, sizeof(data));
        printf("[P1] Envoi: %s\n", msg);
        afficher_horloge();
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
        MessageDataVect data;
        read(client_sock, &data, sizeof(data));

        pthread_mutex_lock(&horloge_mutex);
        for (int i = 0; i < NB_PROCESSUS; i++) {
            if (data.horloge[i] > horloge_vectorielle[i]) {
                horloge_vectorielle[i] = data.horloge[i];
            }
        }
        horloge_vectorielle[PROCESS_ID]++;
        printf("[P1] Réception : %s\n", data.message);
        afficher_horloge();
        pthread_mutex_unlock(&horloge_mutex);

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
    envoyer_message("Message du P1 -> P2: ", 5002);
    sleep(10);
    evenement_local("Local");
    sleep(10);
    envoyer_message("Message du P1 -> P3: ", 5003);
    sleep(10);
    evenement_local("Local");
    sleep(10);
    envoyer_message("Message du P1 -> P4: ", 5004);
    sleep(10);
    evenement_local("Local");
    sleep(10);
    printf("Processus 1 terminé\n");

    return 0;
}
