#include <unistd.h>
#include <zmq.h>
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
            switch (i)
            {
            case 0:
                client_data[i]->pos_y = WINDOW_SIZE / 2;
                client_data[i]->pos_x = 1;
                client_data[i]->movement = 1;
                break;
            case 1:
                client_data[i]->pos_y = 1;
                client_data[i]->pos_x = WINDOW_SIZE / 2;
                client_data[i]->movement = 0;
                break;
            case 2:
                client_data[i]->pos_y = WINDOW_SIZE / 2;
                client_data[i]->pos_x = WINDOW_SIZE - 2;
                client_data[i]->movement = 1;
                break;
            case 3:
                client_data[i]->pos_y = WINDOW_SIZE - 2;
                client_data[i]->pos_x = WINDOW_SIZE / 2;
                client_data[i]->movement = 0;
                break;
            case 4:
                client_data[i]->pos_y = WINDOW_SIZE / 2;
                client_data[i]->pos_x = 2;
                client_data[i]->movement = 1;
                break;
            case 5:
                client_data[i]->pos_y = 2;
                client_data[i]->pos_x = WINDOW_SIZE / 2;
                client_data[i]->movement = 0;
                break;
            case 6:
                client_data[i]->pos_y = WINDOW_SIZE / 2;
                client_data[i]->pos_x = WINDOW_SIZE - 3;
                client_data[i]->movement = 1;
                break;
            case 7:
                client_data[i]->pos_y = WINDOW_SIZE - 3;
                client_data[i]->pos_x = WINDOW_SIZE / 2;
                client_data[i]->movement = 0;
                break;
            }

            (*n_players)++; // Increment player count
            return i; // Return the index of the new player
        }
    }

    // If no free slot was found
    return -1;
}


int handle_astronaut_disconnect(client_info *client_data[], int *n_players, char* client_id, int *pos_x, int *pos_y){
    for (int i = 0; i < 8; i++)
    {
        if (!strcmp(client_data[i]->client_id,client_id)){
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

int handle_astronaut_movement(client_info *client_data[], char *client_id, direction_t direction, int *pos_x, int *pos_y){
    for (int i = 0; i < 8; i++)
    {
        if (!strcmp(client_data[i]->client_id,client_id)){
			if(client_data[i]->stunned == 0){
				(*pos_x) = client_data[i]->pos_x;
				(*pos_y) = client_data[i]->pos_y;

				int new_pos_x = client_data[i]->pos_x;
				int new_pos_y = client_data[i]->pos_y;

				int response;
				response = new_position(&new_pos_x, &new_pos_y, direction, client_data[i]->movement);
				if(response == -1){
					// error ocurred: didnt update position
					return -1;
				}
				else if(response == -2){
					// invalid move: didnt update position
					return -2;
				}
				else if(response == 1){
					// player move: update position
					client_data[i]->pos_x = new_pos_x;
					client_data[i]->pos_y = new_pos_y;

					return i;
				}
			}
			else return -1;
        }
    }
    return -1;
    
}

int handle_astronaut_zap(WINDOW *win, client_info *client_data[], char *client_id, alien bad_guys[ENEMY_NUMBER]){
	int pos_x = -1;
	int pos_y = -1;
    for (int i = 0; i < 8; i++)
    {
        if (!strcmp(client_data[i]->client_id,client_id)){
            pos_x = client_data[i]->pos_x;
            pos_y = client_data[i]->pos_y;

			if(client_data[i]->zap_y != -1 || client_data[i]->zap_x != -1){
				return -1;
			}
			
			client_data[i]->zap_x = pos_x;
			client_data[i]->zap_y = pos_y;
			
			if(client_data[i]->movement == 0 && pos_y > WINDOW_SIZE/2){
				//está à direita
				
				for(int j = 0; j < ENEMY_NUMBER ; j++){
					if(bad_guys[j].pos_x == pos_x){
						if(bad_guys[j].life == 1){
							bad_guys[j].life = 0;
							client_data[i]->points += 10;
						}
					}
				}

				for(int j = 0; j < 8; j++){
					if(client_data[j]->pos_x == pos_x && strcmp(client_id, client_data[j]->client_id)){
						client_data[j]->stunned = 1;
						int pid2 = fork();
						if(pid2 == 0){
							sleep(3);

							void *cntx = zmq_ctx_new();
							void *socket = zmq_socket(cntx, ZMQ_REQ);
							zmq_connect(socket, "tcp://localhost:5555");

							remote_char_t m;
							strcpy(m.client_id, client_data[j]->client_id);
							m.msg_type = 4;				
							zmq_send(socket, &m, sizeof(m), 0);
							char buff[256];
							zmq_recv(socket, buff, sizeof(buff), 0);
							zmq_close(socket);
							zmq_ctx_destroy(cntx);
							exit(0);
						}
					}
				}

				while(pos_y > 1){
					pos_y--;
					char a = mvwinch(win, pos_x, pos_y);
					if(a == ' ')
						waddch(win,'-');
				}
			}

			else if(client_data[i]->movement == 0 && pos_y < WINDOW_SIZE/2){
				//está à esquerda
				for(int j = 0; j < ENEMY_NUMBER ; j++){
					if(bad_guys[j].pos_x == pos_x){
						if(bad_guys[j].life == 1){
							bad_guys[j].life = 0;
							client_data[i]->points += 10;
						}
					}
				}

				for(int j = 0; j < 8; j++){
					if(client_data[j]->pos_x == pos_x && strcmp(client_id, client_data[j]->client_id)){
						client_data[j]->stunned = 1;
						int pid2 = fork();
						if(pid2 == 0){
							sleep(3);

							void *cntx = zmq_ctx_new();
							void *socket = zmq_socket(cntx, ZMQ_REQ);
							zmq_connect(socket, "tcp://localhost:5555");

							remote_char_t m;
							strcpy(m.client_id, client_data[j]->client_id);
							m.msg_type = 4;				
							zmq_send(socket, &m, sizeof(m), 0);
							char buff[256];
							zmq_recv(socket, buff, sizeof(buff), 0);
							zmq_close(socket);
							zmq_ctx_destroy(cntx);
							exit(0);
						}
					}
				}

				while(pos_y  < WINDOW_SIZE){
					pos_y++;
					char a = mvwinch(win, pos_x, pos_y);
					if(a == ' ')
						waddch(win,'-');
				}
			}

			else if(client_data[i]->movement == 1 && pos_x < WINDOW_SIZE/2){
				//está em cima

				for(int j = 0; j < ENEMY_NUMBER ; j++){
					if(bad_guys[j].pos_y == pos_y){
						if(bad_guys[j].life == 1){
							bad_guys[j].life = 0;
							client_data[i]->points += 10;
						}
					}
				}

				for(int j = 0; j < 8; j++){
					if(client_data[j]->pos_y == pos_y && strcmp(client_id, client_data[i]->client_id)){
						client_data[j]->stunned = 1;
						int pid2 = fork();
						if(pid2 == 0){
							sleep(3);

							void *cntx = zmq_ctx_new();
							void *socket = zmq_socket(cntx, ZMQ_REQ);
							zmq_connect(socket, "tcp://localhost:5555");

							remote_char_t m;
							strcpy(m.client_id, client_data[j]->client_id);
							m.msg_type = 4;				
							zmq_send(socket, &m, sizeof(m), 0);
							char buff[256];
							zmq_recv(socket, buff, sizeof(buff), 0);
							zmq_close(socket);
							zmq_ctx_destroy(cntx);
							exit(0);
						}
					}
				}
				while(pos_x  < WINDOW_SIZE){
					pos_x++;
					char a = mvwinch(win, pos_x, pos_y);
					if(a == ' ')
						waddch(win,'|');
				}
			}

			else if(client_data[i]->movement == 1 && pos_x > WINDOW_SIZE/2){
				//está em baixo
				for(int j = 0; j < ENEMY_NUMBER ; j++){
					if(bad_guys[j].pos_y == pos_y){
						if(bad_guys[j].life == 1){
							bad_guys[j].life = 0;
							client_data[i]->points += 10;
						}
					}
				}

				for(int j = 0; j < 8; j++){
					if(client_data[j]->pos_y == pos_y && strcmp(client_id, client_data[i]->client_id)){
						client_data[j]->stunned = 1;
						int pid2 = fork();
						if(pid2 == 0){
							sleep(3);

							void *cntx = zmq_ctx_new();
							void *socket = zmq_socket(cntx, ZMQ_REQ);
							zmq_connect(socket, "tcp://localhost:5555");

							remote_char_t m;
							strcpy(m.client_id, client_data[j]->client_id);
							m.msg_type = 4;				
							zmq_send(socket, &m, sizeof(m), 0);
							char buff[256];
							zmq_recv(socket, buff, sizeof(buff), 0);
							zmq_close(socket);
							zmq_ctx_destroy(cntx);
							exit(0);
						}
					}
				}
				while(pos_x  > 1){
					pos_x--;
					char a = mvwinch(win, pos_x, pos_y);
					if(a == ' ')
						waddch(win,'|');
				}
			}

			int pid = fork();
			if(pid == 0){
				usleep(500000);
				void *cntx = zmq_ctx_new();
				void *socket = zmq_socket(cntx, ZMQ_REQ);
				zmq_connect(socket, "tcp://localhost:5555");

				remote_char_t m;
				strcpy(m.client_id, client_id);
				m.msg_type = 3;				
				zmq_send(socket, &m, sizeof(m), 0);
				char buff[256];
				zmq_recv(socket, buff, sizeof(buff), 0);
				zmq_close(socket);
				zmq_ctx_destroy(cntx);
				exit(0);
			}

		}
    }
	return(0);
}

void handle_astronaut_not_zap(WINDOW *win, client_info *client_data[], char *client_id){
	int pos_x = -1;
	int pos_y = -1;
    for (int i = 0; i < 8; i++)
    {
        if (!strcmp(client_data[i]->client_id,client_id)){
            pos_x = client_data[i]->zap_x;
            pos_y = client_data[i]->zap_y;

			
			if(client_data[i]->movement == 0 && pos_y > WINDOW_SIZE/2){
				//está à direita
				while(pos_y > 1){
					pos_y--;
					char a = mvwinch(win, pos_x, pos_y);
					if(a == '-'|| a == '*')
						waddch(win,' ');
				}
			}

			else if(client_data[i]->movement == 0 && pos_y < WINDOW_SIZE/2){
				//está à esquerda
				while(pos_y  < WINDOW_SIZE){
					pos_y++;
					char a = mvwinch(win, pos_x, pos_y);
					if(a == '-'|| a == '*')
						waddch(win,' ');
				}
			}

			else if(client_data[i]->movement == 1 && pos_x < WINDOW_SIZE/2){
				//está em cima
				while(pos_x  < WINDOW_SIZE){
					pos_x++;
					char a = mvwinch(win, pos_x, pos_y);
					if(a == '|' || a == '*')
						waddch(win,' ');
				}
			}

			else if(client_data[i]->movement == 1 && pos_x > WINDOW_SIZE/2){
				//está em baixo
				while(pos_x  > 1){
					pos_x--;
					char a = mvwinch(win, pos_x, pos_y);
					if(a == '|'|| a == '*')
						waddch(win,' ');
				}
			}
				client_data[i]->zap_x = -1;
				client_data[i]->zap_y = -1;

		}
    }
}
void check_if_client_exists(){

}
