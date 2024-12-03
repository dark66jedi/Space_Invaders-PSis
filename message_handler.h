#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void handle_astronaut_connect(struct client_info** head, int pos_x, int pos_y);
void handle_astronaut_disconnect();
void handle_astronaut_movement();
void handle_astronaut_zap();