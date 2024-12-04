#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "aux_global.h"

int handle_astronaut_connect(client_info* client_data[], int *n_players);
void handle_astronaut_disconnect(client_info *client_data[], int *n_players, char* client_id);
void handle_astronaut_movement();
void handle_astronaut_zap();

#endif