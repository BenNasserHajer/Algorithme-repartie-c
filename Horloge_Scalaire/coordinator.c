#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define PORT_COORDINATOR 5000
#define NB_PROCESSUS 4

int processus_enregistres[NB_PROCESSUS] = {0};
int total_enregistres = 0;

void handle_client(int client_sock) {
    char buffer[128] = {0};
    read(client_sock, buffer, sizeof(buffer));

    if (strncmp(buffer, "REGISTER:", 9) == 0) {
        int id = atoi(buffer + 9);
        if (!processus_enregistres[id]) {
            processus_enregistres[id] = 1;
            total_enregistres++;
            printf("Processus %d enregistré (%d/%d)\n", id + 1, total_enregistres, NB_PROCESSUS);
        }
        if (total_enregistres == NB_PROCESSUS) {
            write(client_sock, "START\n", 6);
        } else {
            write(client_sock, "WAIT\n", 5);
        }
    } else if (strncmp(buffer, "CHECK_STATUS", 12) == 0) {
        if (total_enregistres == NB_PROCESSUS)
            write(client_sock, "START\n", 6);
        else
            write(client_sock, "WAIT\n", 5);
    }

    close(client_sock);
}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_COORDINATOR);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_sock, 5);

    printf("Coordinateur démarré sur le port %d\n", PORT_COORDINATOR);

    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        handle_client(client_sock);
    }

    return 0;
}
