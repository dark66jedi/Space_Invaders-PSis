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
#include "LinkedList.h"

#define WINDOW_SIZE 20

void *context;
WINDOW * my_win;
WINDOW * points;
void *socket_display;
client_info *client_data[8]; // Array of pointers to client_info
LinkedList *bad_guys;
LinkedList *alien_th; 
int running;  // used to close the server for whatever reason
pthread_mutex_t alien_lck = PTHREAD_MUTEX_INITIALIZER;


void serialize_window(WINDOW *win, char *buffer) {
    int idx = 0;
    for (int y = 0; y < WINDOW_SIZE; y++) {
        for (int x = 0; x < WINDOW_SIZE; x++) {
            buffer[idx++] = mvwinch(win, y, x) & A_CHARTEXT; // Get character only
        }
    }
}

void free_alien_th(pthread_t *th){

	pthread_join(*th, NULL);
	free(th);

}


void draw_aliens(){

	pthread_mutex_lock(&alien_lck);
	LinkedList *head = bad_guys;
	while(head != NULL){
		alien *bad_guy = getItemLinkedList(head);

		//right new alien and update
		if(bad_guy->life == 1){
			wmove(my_win, bad_guy->pos_x, bad_guy->pos_y);
			waddch(my_win,'*');
		}

		head = getNextNodeLinkedList(head);
	}
	pthread_mutex_unlock(&alien_lck);
}

void draw_players(){
	for (int i = 0; i < 8; i++)
	{
		if ((client_data[i]->pos_x != -1) && (client_data[i]->pos_y != -1))
		{
			wmove(my_win, client_data[i]->pos_x, client_data[i]->pos_y);
			waddch(my_win,client_data[i]->ch);
		}
		
	}
	
}

void draw_zap(){

	for(int i = 0; i < 8; i++){
		if(client_data[i]->view_zap == 1){
			int pos_x = client_data[i]->zap_x;
			int pos_y = client_data[i]->zap_y;

			if(client_data[i]->movement == 0 && pos_y > WINDOW_SIZE/2){
				//Está à direita
				while(pos_y > 1){
					pos_y--;
					char a = mvwinch(my_win, pos_x, pos_y);
					if(a == ' ')
						waddch(my_win,'-');
				}
			}
			else if(client_data[i]->movement == 0 && pos_y < WINDOW_SIZE/2){
				//Está à esquerda
				while(pos_y  < WINDOW_SIZE){
					pos_y++;
					char a = mvwinch(my_win, pos_x, pos_y);
					if(a == ' ')
						waddch(my_win,'-');
				}
			}

			else if(client_data[i]->movement == 1 && pos_x < WINDOW_SIZE/2){
				//Está em cima

				while(pos_x  < WINDOW_SIZE){
					pos_x++;
					char a = mvwinch(my_win, pos_x, pos_y);
					if(a == ' ')
						waddch(my_win,'|');
				}
			}

			else if(client_data[i]->movement == 1 && pos_x > WINDOW_SIZE/2){
				//Está em baixo
				while(pos_x  > 1){
					pos_x--;
					char a = mvwinch(my_win, pos_x, pos_y);
					if(a == ' ')
						waddch(my_win,'|');
				}
			}
		}
	}
}

void *window_thread(void *){
	void *socket_display = zmq_socket(context, ZMQ_PUB);
	zmq_bind(socket_display, "tcp://*:5556"); // Bind to TCP port 5556
	while(running){	
		usleep(10000);	
		werase(my_win);
		box(my_win, 0 , 0);
		draw_aliens();
		draw_players();
		draw_zap();
		wrefresh(my_win);

		/* draw points*/
		werase(points);
		box(points, 0 , 0);
		update_points_display(points, client_data);
		wrefresh(points);

		char win_buffer[WINDOW_SIZE * WINDOW_SIZE];
		serialize_window(my_win, win_buffer);
		zmq_send(socket_display, &win_buffer, sizeof(win_buffer), 0);
		serialize_window(points, win_buffer);
		zmq_send(socket_display, &win_buffer, sizeof(win_buffer), 0);
	}
}

void *closing_thread(void *){
	while(running){	
		int key = getch();

		if(key == 'Q'){
			running = 0;
			remote_char_t m;
			m.msg_type = -2; // message type to stop server
			void *socket = zmq_socket(context, ZMQ_REQ);
    		zmq_connect(socket, "tcp://localhost:5555");
			zmq_send(socket, &m, sizeof(remote_char_t), 0);
			char buffer[256];
    		zmq_recv(socket, buffer, 255, 0);
		}
	}
}

void *alien_thread(alien *bad_guy){
	while(running){
		while(bad_guy->life == 1){
			sleep(1);

			//update aliens
			switch(bad_guy->movement){
				case UP:
					bad_guy->pos_y--;
					if(bad_guy->pos_y < 3)
						bad_guy->pos_y = 3;
				break;
				case DOWN:
					bad_guy->pos_y++;
					if(bad_guy->pos_y > 16)
						bad_guy->pos_y = 16;
				break;
				case LEFT:
					bad_guy->pos_x--;
					if(bad_guy->pos_x < 3)
						bad_guy->pos_x = 3;
				break;
				case RIGHT:
					bad_guy->pos_x++;
					if(bad_guy->pos_x > 16)
						bad_guy->pos_x = 16;
				break;
			}

			bad_guy->movement = rand() %4;

		}



		pthread_mutex_lock(&alien_lck);
		LinkedList *head = bad_guys;
		LinkedList *aux = head;
		if(head != NULL){
			if(getItemLinkedList(head) == bad_guy){
				aux = getNextNodeLinkedList(head);
				free(bad_guy);
				free(head);
				bad_guys = aux;
				pthread_mutex_unlock(&alien_lck);
				return NULL;
			}
		}

		while(head != NULL){
			aux = getNextNodeLinkedList(head);
			if(aux == NULL) break;

			if(getItemLinkedList(aux) == bad_guy){
				revoveFromList(head, aux, free);
				break;
			}
			head = aux;
		}
		pthread_mutex_unlock(&alien_lck);
	}
	return NULL;
}

void *spawn_thread(void *){
	pthread_mutex_lock(&alien_lck);
	int n = lengthLinkedList(bad_guys);
	pthread_mutex_unlock(&alien_lck);
	int m = n;
	while(running){
		sleep(10);
		pthread_mutex_lock(&alien_lck);
		n = lengthLinkedList(bad_guys);
		pthread_mutex_unlock(&alien_lck);
		if(n == m){
			for (float i = m*0.1f; i > 0; i--){
				alien *bad_guy = (alien *) malloc(sizeof(alien));
				pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t));

				bad_guy->pos_x = (rand() % (WINDOW_SIZE-6)) + 3;
				bad_guy->pos_y = (rand() % (WINDOW_SIZE-6)) + 3;
				bad_guy->movement = rand() % 4;
				bad_guy->life = 1;

				pthread_create(thread, NULL, (void *(*)(void*)) alien_thread, bad_guy);
				pthread_mutex_lock(&alien_lck);
				bad_guys = insertUnsortedLinkedList(bad_guys, (Item) bad_guy);
				n = lengthLinkedList(bad_guys);
				alien_th = insertUnsortedLinkedList(alien_th, (Item) thread);
				pthread_mutex_unlock(&alien_lck);
			}
		}
		m=n;
	}
}

void free_aliens(){
	LinkedList *head = bad_guys;
	alien *bad_guy;
	
	pthread_mutex_lock(&alien_lck);
	while(head != NULL){
		bad_guy = getItemLinkedList(head);
		bad_guy->life = 0;
		head = getNextNodeLinkedList(head);
	}
	pthread_mutex_unlock(&alien_lck);
}


int main()
{	
	running = 1;

	int check_init;
	check_init = init_client_array(client_data);
	if (check_init == -1){
		perror("Couldnt initialize client array");
		exit(-1);
	}

	int n_players = 0;

	remote_char_t msg;
	remote_char_t m;

	// Create a context
	context = zmq_ctx_new();

	// Create a REP socket
	void *socket_client = zmq_socket(context, ZMQ_REP);
	zmq_bind(socket_client, "tcp://*:5555"); // Bind to TCP port 5555

	char child_id[17];
	generate_client_id(child_id);

	
	//Initial aliens and threads
	
	bad_guys = initLinkedList();
	alien_th = initLinkedList();

	for(int i = 0; i < ENEMY_NUMBER; i++){
		alien *bad_guy = (alien *) malloc(sizeof(alien));
		pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t));

		bad_guy->pos_x = (rand() % (WINDOW_SIZE-6)) + 3;
		bad_guy->pos_y = (rand() % (WINDOW_SIZE-6)) + 3;
		bad_guy->movement = rand() % 4;
		bad_guy->life = 1;

		pthread_create(thread, NULL, (void *(*)(void*)) alien_thread, bad_guy);
		bad_guys = insertUnsortedLinkedList(bad_guys, (Item) bad_guy);
		alien_th = insertUnsortedLinkedList(alien_th, (Item) thread);

	}

	// void *socket_score = zmq_socket(context, ZMQ_PUB);
	// zmq_bind(socket_score, "tcp://*:5557"); // Bind to TCP port 5557

	//curses init
	initscr();
	cbreak();
	curs_set(0);
	keypad(stdscr, TRUE);
	noecho();

	/* creates windows */
	my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
	points = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, WINDOW_SIZE+5);

	int client_idx;
	char reply[256];
	int delete_pos_x, delete_pos_y;

	pthread_t window_th, close_th, spawn_th;
	pthread_create(&window_th, NULL, window_thread, NULL);
	pthread_create(&close_th, NULL, closing_thread, NULL);
	pthread_create(&spawn_th, NULL, spawn_thread, NULL);

	while (running)
	{

		zmq_recv(socket_client, &msg, sizeof(msg), 0);

		// server disconnect
		if (msg.msg_type == -2){
			zmq_send(socket_client, reply, strlen(reply)+1, 0);
		}

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
		}

		// astronaut_connect
		if(msg.msg_type == 0){
			client_idx = handle_astronaut_connect(client_data, &n_players);
			if(client_idx == -1){
				strcpy(reply, "Maximum number of players reached");
				zmq_send(socket_client, reply, strlen(reply)+1, 0);
			}else if(client_idx>=0 && client_idx<=7){
				zmq_send(socket_client, client_data[client_idx]->client_id, strlen(client_data[client_idx]->client_id)+1, 0);
			}else{
				strcpy(reply, "An error occurred");
				zmq_send(socket_client, reply, strlen(reply)+1, 0);
			}
		}
		if(msg.msg_type == 1){
			direction_t direction = msg.value.direction;
			if(strlen(msg.client_id) > 16)
				msg.client_id[16] = '\0';

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
			}
			else{
				// error ocurred: didnt update position
				strcpy(reply, "An error ocurred: position not updated");
				zmq_send(socket_client, reply, strlen(reply)+1, 0);
			}
		}
		if(msg.msg_type == 2){
			if(strlen(msg.client_id) > 16)
				msg.client_id[16] = '\0';

			pthread_t zap_th;
			pthread_create(&zap_th, NULL, (void *(*)(void*)) handle_astronaut_zap, msg.client_id);
			strcpy(reply, "Enemy zapped!");
			zmq_send(socket_client, reply, strlen(reply)+1, 0);
		}


	}
	free_aliens();
	freeLinkedList(alien_th,(void (*)(void *)) free_alien_th);
	freeLinkedList(bad_guys, free);
	pthread_join(window_th, NULL);
	pthread_join(close_th, NULL);

	endwin();			/* End curses mode		  */

	int linger = 0;
    zmq_setsockopt(socket_client, ZMQ_LINGER, &linger, sizeof(linger));
	zmq_close(socket_client);
	zmq_setsockopt(socket_display, ZMQ_LINGER, &linger, sizeof(linger));
	zmq_close(socket_display);
	zmq_ctx_destroy(context);

	return 0;
}
