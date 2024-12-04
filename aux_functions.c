#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "aux_global.h"

void generate_client_id(char* client_id) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    size_t charsetSize = sizeof(charset) - 1; // Exclude the null terminator
    int length = 16;

    for (int i = 0; i < length; i++) {
        client_id[i] = charset[rand() % charsetSize];
    }
    client_id[length] = '\0';
}

