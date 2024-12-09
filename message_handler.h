#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include "aux_global.h"

int handle_astronaut_connect(client_info* client_data[], int *n_players);
int handle_astronaut_disconnect(client_info *client_data[], int *n_players, char* client_id, int *pos_x, int *pos_y);
int handle_astronaut_movement(client_info *client_data[], char *client_id, direction_t direction, int *pos_x, int *pos_y);
int handle_astronaut_zap(WINDOW *win, client_info *client_data[], char *client_id);
void handle_astronaut_not_zap(WINDOW *win, client_info *client_data[], char *client_id);

#endif
