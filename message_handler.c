#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "message_handler.h"
#include "aux_global.h"

int handle_astronaut_connect(client_info *client_data[], int *n_players) {
    for (int i = 0; i < 8; i++) {
        if (!strcmp(client_data[i]->client_id,"----------------")) {
            generate_client_id(client_data[i]->client_id); // Assign a unique client_id
            client_data[i]->ch = 65 + i; // Assign a character ('A' + i)

            // Set initial positions based on the index
            if (i % 2 == 0) {
                client_data[i]->pos_y = WINDOW_SIZE / 2;
                client_data[i]->pos_x = (i < 4) ? 1 : 2;
                client_data[i]->movement=0;
            } else {
                client_data[i]->pos_x = WINDOW_SIZE / 2;
                client_data[i]->pos_y = (i < 4) ? 1 : 2;
                client_data[i]->movement=1;
            }

            (*n_players)++; // Increment player count
            return i; // Return the index of the new player
        }
    }

    // If no free slot was found
    fprintf(stderr, "No free slot available for a new player.\n");
    return -1;
}


int handle_astronaut_disconnect(client_info *client_data[], int *n_players, char* client_id, int *pos_x, int *pos_y){
    for (int i = 0; i < 8; i++)
    {
        if (!strcmp(client_data[i]->client_id,client_id)){
            // printf("Client about to disconnect: %c\n", client_data[i]->ch);
            client_data[i]->ch = '\0';
            strcpy(client_data[i]->client_id,"----------------");
            client_data[i]->movement = -1;

            (*pos_x) = client_data[i]->pos_x;
            (*pos_y) = client_data[i]->pos_y;
            client_data[i]->pos_x = -1;
            client_data[i]->pos_y = -1;

            (*n_players)--;
            return 1;
        }
    }
    return -1;
}

void handle_astronaut_movement(){
    
}

void handle_astronaut_zap(){
    
}

void check_if_client_exists(){

}