#include <ncurses.h>
#include "remote_char.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#define WINDOW_SIZE 20

void deserialize_window(WINDOW *win, char *buffer) {
    int idx = 0;
    for (int y = 0; y < WINDOW_SIZE; y++) {
        for (int x = 0; x < WINDOW_SIZE; x++) {
            mvwaddch(win, y, x, buffer[idx++]);
        }
    }
}

int main()
{	
    // Create a context
    void *context = zmq_ctx_new();

    // Create a REP socket
    void *socket = zmq_socket(context, ZMQ_REP);
    zmq_bind(socket, "tcp://*:5556"); // Bind to TCP port 5556

	initscr();		    	
	cbreak();				
    keypad(stdscr, TRUE);   
	noecho();			    

    /* creates a window and draws a border */
    WINDOW *my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);

    while (1)
    {
        char buffer[WINDOW_SIZE * WINDOW_SIZE];
        zmq_recv(socket, &buffer, sizeof(buffer), 0);
        deserialize_window(my_win, buffer);
        box(my_win, 0 , 0);
        wrefresh(my_win);

        const char *reply = "Reply from server";
        zmq_send(socket, reply, strlen(reply)+1, 0);			
    }
  	endwin();
    zmq_close(socket);
    zmq_ctx_destroy(context);

	return 0;
}