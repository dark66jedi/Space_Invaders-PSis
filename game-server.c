#include <ncurses.h>
#include "remote_char.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdlib.h>
#include <string.h>
#include <zmq.h>
#include "message_handler.h"
#include "aux_global.h"

#define WINDOW_SIZE 20


direction_t random_direction(){
    return  random()%4;

}
void new_position(int* x, int *y, direction_t direction){
    switch (direction)
    {
    case UP:
        (*x) --;
        if(*x ==0)
            *x = 2;
        break;
    case DOWN:
        (*x) ++;
        if(*x ==WINDOW_SIZE-1)
            *x = WINDOW_SIZE-3;
        break;
    case LEFT:
        (*y) --;
        if(*y ==0)
            *y = 2;
        break;
    case RIGHT:
        (*y) ++;
        if(*y ==WINDOW_SIZE-1)
            *y = WINDOW_SIZE-3;
        break;
    default:
        break;
    }
}

void serialize_window(WINDOW *win, char *buffer) {
    int idx = 0;
    for (int y = 0; y < WINDOW_SIZE; y++) {
        for (int x = 0; x < WINDOW_SIZE; x++) {
            buffer[idx++] = mvwinch(win, y, x) & A_CHARTEXT; // Get character only
        }
    }
}

int main()
{	
    //STEP 2
    client_info *client_data[8]; // Array of pointers to client_info
    int check_init;
    check_init = init_client_array(client_data);
    if (check_init == -1){
        perror("Couldnt initialize client array");
        exit(-1);
    }

    int n_players = 0;

    remote_char_t m;

    // Create a context
    void *context = zmq_ctx_new();

    // Create a REP socket
    void *socket_client = zmq_socket(context, ZMQ_REP);
    zmq_bind(socket_client, "tcp://*:5555"); // Bind to TCP port 5555

    void *socket_display = zmq_socket(context, ZMQ_PUB);
    zmq_connect(socket_display, "tcp://localhost:5556"); // Connect to display

	initscr();
	cbreak();
    keypad(stdscr, TRUE);
	noecho();

    /* creates a window and draws a border */
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);

    int client_idx;
    char reply[256];
    direction_t  direction;
    while (1)
    {
        // send to display
        char buffer[WINDOW_SIZE * WINDOW_SIZE];
        serialize_window(my_win, buffer);
        zmq_send(socket_display, &buffer, sizeof(buffer), 0);

        zmq_recv(socket_client, &m, sizeof(m), 0);

        // astronaut_disconnect
        if (m.msg_type == -1){
            int delete_pos_x, delete_pos_y;
            // printf("\nNumber of players before disconnect: %d\n", n_players);
            client_idx = handle_astronaut_disconnect(client_data, &n_players, m.client_id, &delete_pos_x, &delete_pos_y);
            
            if(client_idx == -1){
                strcpy(reply, "Client not disconnected");
            }else if(client_idx == 1){
                strcpy(reply, "Client disconnected");
                wmove(my_win, delete_pos_x, delete_pos_y);
                waddch(my_win,' ');
            }
            zmq_send(socket_client, reply, strlen(reply)+1, 0);
            // printf("Number of players after disconnect: %d\n", n_players);
        }
        
        // astronaut_connect
        if(m.msg_type == 0){
            client_idx = handle_astronaut_connect(client_data, &n_players);
            if(client_idx == -1){
                strcpy(reply, "Maximum number of players reached");
                zmq_send(socket_client, reply, strlen(reply)+1, 0);
            }else if(client_idx>=0 && client_idx<=7){
                // printf("\nCHAR: %c\tCLIENT_ID: %s\tIDX: %d\t\n",client_data[client_idx]->ch,client_data[client_idx]->client_id, client_idx);
                zmq_send(socket_client, client_data[client_idx]->client_id, strlen(client_data[client_idx]->client_id)+1, 0);
                wmove(my_win, client_data[client_idx]->pos_x, client_data[client_idx]->pos_y);
                waddch(my_win,client_data[client_idx]->ch| A_BOLD);
            }else{
                strcpy(reply, "An error occurred");
                zmq_send(socket_client, reply, strlen(reply)+1, 0);
            }
        }
        if(m.msg_type == 1){
            //STEP 4
            // int ch_pos = find_ch_info(head, n_players, m.ch);
            // if(ch_pos != -1){
            //     pos_x = head[ch_pos].pos_x;
            //     pos_y = head[ch_pos].pos_y;
            //     ch = head[ch_pos].ch;
            //     /*deletes old place */
            //     wmove(my_win, pos_x, pos_y);
            //     waddch(my_win,' ');

            //     /* claculates new direction */
            //     direction = m.direction;

            //     /* claculates new mark position */
            //     new_position(&pos_x, &pos_y, direction);
            //     head[ch_pos].pos_x = pos_x;
            //     head[ch_pos].pos_y = pos_y;

            // }        
        }
        /* draw mark on new position */
        

        wrefresh(my_win);
    }
  	// endwin();			/* End curses mode		  */
    zmq_close(socket_client);
    zmq_close(socket_display);
    zmq_ctx_destroy(context);

	return 0;
}