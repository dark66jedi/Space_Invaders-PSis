#ifndef AUX_H
#define AUX_H

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include "LinkedList.h"
#include "scores.pb-c.h"


#define WINDOW_SIZE 20
#define ENEMY_NUMBER 85


extern void *context;
extern WINDOW * my_win;
extern WINDOW * points;
extern LinkedList *bad_guys;

typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

typedef struct client_info
{
    char ch;
    int pos_x, pos_y;
    int movement; // if 0 vertical, if 1 horizontal
    char *client_id;
	int zap_x, zap_y;
	int view_zap;
	int points;
	int stunned;
} client_info;

extern client_info *client_data[8]; // Array of pointers to client_info
									
typedef struct alien{
	int pos_x, pos_y;
	direction_t movement;
	int life;
}alien;

union content{
	direction_t direction;
	alien vect[ENEMY_NUMBER];
};

typedef struct remote_char_t
{   
    int msg_type; /* 0 join   1 - move */
    char client_id[17]; 
    union content value;
}remote_char_t;

void generate_client_id(char* client_id);
int init_client_array(client_info *client_data[]);
int new_position(int* x, int *y, direction_t direction, int allowed_mov);
void update_points_display(WINDOW *points_display, client_info *client_data[]);

#endif
