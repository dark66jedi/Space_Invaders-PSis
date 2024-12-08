#include <ncurses.h>
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

	remote_char_t msg;

	// Create a context
	void *context = zmq_ctx_new();

	// Create a REP socket
	void *socket_client = zmq_socket(context, ZMQ_REP);
	zmq_bind(socket_client, "tcp://*:5555"); // Bind to TCP port 5555
	
	char child_id[17];
	generate_client_id(child_id);


	remote_char_t m;
	m.msg_type = 5;
	strcpy(m.client_id, child_id);
	alien *bad_guys = m.value.vect;
	for(int i = 0; i < ENEMY_NUMBER; i++){
		bad_guys[i].pos_x = rand() % WINDOW_SIZE;
		bad_guys[i].pos_y = rand() % WINDOW_SIZE;
		bad_guys[i].movement = rand() % 4;
		bad_guys[i].life = 1;
		printf("Enemy number %d:\n X: %d\n Y: %d\n Mov: %d\n Life: %d\n", i, bad_guys[i].pos_x, bad_guys[i].pos_y, bad_guys[i].movement, bad_guys[i].life);
	}

    int pid = fork();
    if(pid == 0){ //child code
		// Create a context
		void *context = zmq_ctx_new();

    	void *socket_child = zmq_socket(context, ZMQ_REQ);
		//sleep(1);
    	zmq_connect(socket_child, "tcp://localhost:5555");
		
		zmq_send(socket_child, 	&m, sizeof(m), 0);
		char buffer[256];
		zmq_recv(socket_child, buffer, sizeof(buffer), 0 );

    	do{
    		sleep(1);

    		for(int i = 0; i < ENEMY_NUMBER; i++){
    			//update aliens
				switch(bad_guys[i].movement){

					case UP:
						bad_guys[i].pos_y++;
						break;
					case DOWN:
						bad_guys[i].pos_y--;
						break;
					case LEFT:
						bad_guys[i].pos_x--;
						break;
					case RIGHT:
						bad_guys[i].pos_x++;
						break;
				}

    			bad_guys[i].movement = rand() %4;
    		}

    		m.msg_type = 5;
    		zmq_send(socket_child, &m, sizeof(m), 0);
			zmq_recv(socket_child, buffer, sizeof(buffer), 0 );

    	} while(1);

    } else{ //parrent code

		void *socket_display = zmq_socket(context, ZMQ_PUB);
		zmq_connect(socket_display, "tcp://localhost:5556"); // Connect to display

		//curses init
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
		int delete_pos_x, delete_pos_y;
		while (1)
		{
			// send to display
			char buffer[WINDOW_SIZE * WINDOW_SIZE];
			serialize_window(my_win, buffer);
			zmq_send(socket_display, &buffer, sizeof(buffer), 0);

			zmq_recv(socket_client, &msg, sizeof(msg), 0);
			// printf("Received msg_type: %d\n", msg.msg_type);
			// printf("Received direction: %d\n", msg.direction);
			// printf("Received client_id: %s\n", msg.client_id);

			// astronaut_disconnect
			if (msg.msg_type == -1){
				if(strlen(msg.client_id) > 16)
					msg.client_id[16] = '\0';

				client_idx = handle_astronaut_disconnect(client_data, &n_players, msg.client_id, &delete_pos_x, &delete_pos_y);

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
			if(msg.msg_type == 0){
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
			if(msg.msg_type == 1){
				direction_t direction = msg.value.direction;
				if(strlen(msg.client_id) > 16)
					msg.client_id[16] = '\0';

				// mvprintw(2, 25, "msg_type number %d", m.msg_type);

				client_idx = handle_astronaut_movement(client_data, msg.client_id, direction, &delete_pos_x, &delete_pos_y);

				if(client_idx == -1){
					// error ocurred: didnt update position
					strcpy(reply, "An error ocurred: position not updated");
					zmq_send(socket_client, reply, strlen(reply)+1, 0);
				}
				else if(client_idx == -2){
					// invalid move: didnt update position
					strcpy(reply, "Invalid move: position not updated");
					zmq_send(socket_client, reply, strlen(reply)+1, 0);
				}
				else if(client_idx>=0 && client_idx<=7){
					// player move: update position
					strcpy(reply, "Player moved: position updated");
					zmq_send(socket_client, reply, strlen(reply)+1, 0);

					wmove(my_win, delete_pos_x, delete_pos_y);
					waddch(my_win,' ');

					wmove(my_win, client_data[client_idx]->pos_x, client_data[client_idx]->pos_y);
					waddch(my_win,client_data[client_idx]->ch| A_BOLD);

				}
				else{
					// error ocurred: didnt update position
					strcpy(reply, "An error ocurred: position not updated");
					zmq_send(socket_client, reply, strlen(reply)+1, 0);
				}
			}
	    	if(msg.msg_type == 5){

				if(strlen(msg.client_id) > 16)
					msg.client_id[16] = '\0';

				strcpy(reply, "Aliens updated");
				zmq_send(socket_client, reply, strlen(reply)+1, 0);

	    		if(!strcmp(msg.client_id, child_id)){
	    			for(int i = 0; i < ENEMY_NUMBER; i++){
	    				//delete previous	
	    				wmove(my_win, bad_guys[i].pos_x, bad_guys[i].pos_y);
	    				waddch(my_win,' ');

	    				bad_guys[i].pos_x = msg.value.vect[i].pos_x ;
						bad_guys[i].pos_y  = msg.value.vect[i].pos_y ;
	    				bad_guys[i].life = msg.value.vect[i].life;

	    				if(msg.value.vect[i].life == 1){
	    					//right new alien and update
	    					wmove(my_win, msg.value.vect[i].pos_x, msg.value.vect[i].pos_y);
	    					waddch(my_win,'*');
	    				}
	    			}
	    		}
	    		else{
	    			printf("We detected an unallowed attemped to manipulate the aliens\n");
	    			exit(1);
	    		}
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
}
