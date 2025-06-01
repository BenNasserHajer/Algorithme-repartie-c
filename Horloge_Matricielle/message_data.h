#ifndef MESSAGE_DATA_H
#define MESSAGE_DATA_H

#define NB_PROCESSUS 4
#define MAX_MESSAGE_LEN 256

typedef struct {
    char message[MAX_MESSAGE_LEN];
    int horloge[NB_PROCESSUS][NB_PROCESSUS];
} MessageData;

#endif
