#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aux_global.h"

void generate_client_id(char* client_id) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    size_t charsetSize = sizeof(charset) - 1; // Exclude the null terminator
    int length = 16;
	client_id[16] = '\0';

    for (int i = 0; i < length; i++) {
        client_id[i] = charset[rand() % charsetSize];
    }
}

int init_client_array(client_info *client_data[]){
    for (int i = 0; i < 8; i++)
    {
        client_data[i] = malloc(sizeof(client_info));
        if (client_data[i] == NULL) {
            perror("Failed to allocate memory for client_data[i]");
            return -1; // Error
        }
        
        client_data[i]->client_id = malloc(17); // Adjust size as necessary
        if (client_data[i]->client_id == NULL) {
            perror("Failed to allocate memory for client_id");
            free(client_data[i]); // Free allocated memory
            client_data[i] = NULL;
            return -1; // Error
        }

        client_data[i]->ch = '\0';
        strcpy(client_data[i]->client_id,"----------------");
        client_data[i]->movement = -1;
        client_data[i]->pos_x = -1;
        client_data[i]->pos_y = -1;
        client_data[i]->points = 0;
        client_data[i]->zap_x = -1;
        client_data[i]->zap_y = -1;
		client_data[i]->view_zap = 0;
		client_data[i]->stunned = 0;

    }
    return 1;
}

int new_position(int* x, int *y, direction_t direction, int allowed_mov){
    if (allowed_mov == 0)
    {
        if (direction == UP)
        {
            (*x) --;
            if(*x ==2)
                *x = 3;
            return 1;
        }
        else if(direction == DOWN){
            (*x) ++;
            if(*x ==WINDOW_SIZE-3)
                *x = WINDOW_SIZE-4;
            return 1;
        }
        else{
            return -2;
        }
    }
    else if (allowed_mov == 1){
        if (direction == LEFT)
        {
            (*y) --;
            if(*y ==2)
                *y = 3;
            return 1;
        }
        else if(direction == RIGHT){
            (*y) ++;
            if(*y ==WINDOW_SIZE-3)
                *y = WINDOW_SIZE-4;
            return 1;
        }
        else{
            return -2;
        }
    }
    return -1;
}

void update_points_display(WINDOW *points_display, client_info *client_data[]){

    mvwprintw(points_display, 2, 2, "SCORE:");

    for (int i = 0; i < 8; i++)
    {
        mvwprintw(points_display, i+5, 2, "          ");
        if(strcmp(client_data[i]->client_id, "----------------")){
            mvwprintw(points_display, i+5, 2, "%c - %d", client_data[i]->ch, client_data[i]->points);
        }
    }
    
}
