#include <ncurses.h>
#include "aux_global.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <zmq.h>
#include <string.h>

int main()
{
	// Create a context
	void *context = zmq_ctx_new();

	// Create a REQ socket
	void *socket = zmq_socket(context, ZMQ_REQ);
	zmq_connect(socket, "tcp://localhost:5555"); // Connect to server

	// TODO_6
	// send connection message
	remote_char_t m;
	m.msg_type = 0;
	zmq_send(socket, &m, sizeof(m), 0);
	char buffer[256];
	zmq_recv(socket, buffer, 255, 0);

	// TODO Adicionar disconnects 
	if (!strcmp(buffer, "Maximum number of players reached")){
		printf("%s /n", buffer);
		zmq_close(socket);
		zmq_ctx_destroy(context);
		return -1;
	}
	else if (!strcmp(buffer, "An error occurred")){
		printf("%s /n", buffer);
		zmq_close(socket);
		zmq_ctx_destroy(context);
		return -1;
	}
	else
	{
		strcpy(m.client_id, buffer);
	}

	initscr();            /* Start curses mode 		*/
	cbreak();             /* Line buffering disabled	*/
	keypad(stdscr, TRUE); /* We get F1, F2 etc..		*/
	noecho();             /* Don't echo() while we do getch */

	int n = 0;

	// TODO_9
	//  prepare the movement message
	m.msg_type = 1;

	int key;
	do
	{
		key = getch();
		n++;
		switch (key)
		{
			case 'q':
			case 'Q':
				m.msg_type = -1;
				break;
			case KEY_LEFT:
				m.value.direction = LEFT;
				m.msg_type = 1;
				break;
			case KEY_RIGHT:
				m.value.direction = RIGHT;
				m.msg_type = 1;
				break;
			case KEY_DOWN:
				m.msg_type = 1;
				m.value.direction = DOWN;
				break;
			case KEY_UP:
				m.msg_type = 1;
				m.value.direction = UP;
				break;
			case ' ':
				m.msg_type = 2;
				break;

			default:
				key = 'x';
				break;
		}

		// TODO_10
		//  send the movement message
		if (key != 'x')
		{
			zmq_send(socket, &m, sizeof(remote_char_t), 0);
			zmq_recv(socket, buffer, 255, 0);
			if (!strcmp(buffer, "Client disconnected")) break;
			else if (!strcmp(buffer, "Client not disconnected")){
				printf("%s. Try again",buffer);
			}
		}
		refresh(); /* Print it on to the real screen */
	} while (key != 27);

	endwin(); /* End curses mode		  */
	printf("%s\n", buffer);
	// Clean up
	zmq_close(socket);
	zmq_ctx_destroy(context);

	return 0;
}
