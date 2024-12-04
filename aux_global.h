#ifndef AUX_H
#define AUX_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define WINDOW_SIZE 20

typedef struct client_info
{
    char ch;
    int pos_x, pos_y;
    int movement; // if 0 vertical, if 1 horizontal
    char *client_id;
} client_info;

void generate_client_id(char* client_id);

#endif