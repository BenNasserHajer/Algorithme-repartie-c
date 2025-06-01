#ifndef MESSAGE_DATA_H
#define MESSAGE_DATA_H

#define MAX_MESSAGE_LEN 256

typedef struct {
    char message[MAX_MESSAGE_LEN];
    int horloge;
    int processus_id;
} MessageData;

#endif
