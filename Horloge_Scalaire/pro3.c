#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "message_data.h"  

#define PROCESS_ID 2
#define PROCESS_PORT 5003

int horloge = 0;
int compteur_evenements = 0;
pthread_mutex_t horloge_mutex = PTHREAD_MUTEX_INITIALIZER;


void afficher_horloge(const char *prefix, const char *desc, int numero)
 {
    printf("[P3] %s #%d (H=%d): %s\n", prefix, numero, horloge, desc);
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
    horloge++;
    compteur_evenements++;
    afficher_horloge("Événement local", desc, compteur_evenements);
    pthread_mutex_unlock(&horloge_mutex);
}

void envoyer_message(const char *msg, int port) {
    pthread_mutex_lock(&horloge_mutex);
    horloge++;
    int h = horloge;

    MessageData data;
    strncpy(data.message, msg, MAX_MESSAGE_LEN);
    data.horloge = h;
    data.processus_id = PROCESS_ID;
    pthread_mutex_unlock(&horloge_mutex);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    dest.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&dest, sizeof(dest)) == 0) {
        write(sock, &data, sizeof(data));
        printf("[P3] Envoi à P%d: %s (H=%d)\n", port - 5000, msg, h);
        printf("\n");
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

        pthread_mutex_lock(&horloge_mutex);
        horloge = (horloge > data.horloge ? horloge : data.horloge) + 1;
        printf("[P3] Réception de P%d: %s (H=%d)\n", data.processus_id + 1, data.message, horloge);
        printf("\n");
        pthread_mutex_unlock(&horloge_mutex);

        close(client_sock);
    }

    return NULL;
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, reception_thread, NULL);

    if (!attendre_demarrage()) {
        printf("Erreur de démarrage.\n");
        return 1;
    }

    sleep(1);
    evenement_local("Local");
    sleep(10);
    envoyer_message("Message du P3 -> P2: ", 5002);
    sleep(10);
    evenement_local("Local");
    sleep(10);
    envoyer_message("Message du P3 -> P1: ", 5001);
    sleep(10);
    evenement_local("Local");
    sleep(10);
    envoyer_message("Message du P3 -> P4: ", 5004);
    sleep(10);
    evenement_local("Local ");
    sleep(10);

    printf("Processus 3 terminé\n");
    return 0;
}
