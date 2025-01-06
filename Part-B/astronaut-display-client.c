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
#include <pthread.h>

// Flag to stop client
int keep_running = 1;

#define WINDOW_SIZE 20

void deserialize_window(WINDOW *win, char *buffer) {
    int idx = 0;
    for (int y = 0; y < WINDOW_SIZE; y++) {
        for (int x = 0; x < WINDOW_SIZE; x++) {
            mvwaddch(win, y, x, buffer[idx++]);
        }
    }
}

// Thread function to display the board
void *update_display_thread(void *arg)
{
    void *context = zmq_ctx_new();

    // Create a REP socket
    void *socket = zmq_socket(context, ZMQ_SUB);
	zmq_connect(socket, "tcp://localhost:5556");
    zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);			    

    /* creates a window and draws a border */
    WINDOW *my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    WINDOW *points_display = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 25);

    while (keep_running)
    {
        char buffer[WINDOW_SIZE * WINDOW_SIZE];
        zmq_recv(socket, &buffer, sizeof(buffer), 0);
        deserialize_window(my_win, buffer);
        zmq_recv(socket, &buffer, sizeof(buffer), 0);
        deserialize_window(points_display, buffer);
        box(my_win, 0 , 0);
        wrefresh(my_win);
        box(points_display, 0 , 0);
        wrefresh(points_display);
    }
    zmq_close(socket);
    zmq_ctx_destroy(context);
    return NULL;
}

int main()
{
    // Create a context and socket
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_REQ);
    zmq_connect(socket, "tcp://localhost:5555");

    remote_char_t m;
    m.msg_type = 0;
    zmq_send(socket, &m, sizeof(m), 0);

    char buffer[256];
    zmq_recv(socket, buffer, 255, 0);

    if (!strcmp(buffer, "Maximum number of players reached") ||
        !strcmp(buffer, "An error occurred"))
    {
        printf("%s\n", buffer);
        zmq_close(socket);
        zmq_ctx_destroy(context);
        return -1;
    }
    else
    {
        strcpy(m.client_id, buffer);
    }

    initscr();		    	
    cbreak();				
    keypad(stdscr, TRUE);   
    noecho();

    pthread_t display_thread;

    pthread_create(&display_thread, NULL, update_display_thread, NULL);

    int key;
    while (keep_running)
    {
        key = getch();
        switch (key)
        {
        case 'q':
        case 'Q':
            m.msg_type = -1;
            keep_running = 0;
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
            m.value.direction = DOWN;
            m.msg_type = 1;
            break;
        case KEY_UP:
            m.value.direction = UP;
            m.msg_type = 1;
            break;
        case ' ':
            m.msg_type = 2;
            break;
        default:
            continue;
        }

        if (key != 'x')
        {
            zmq_send(socket, &m, sizeof(remote_char_t), 0);
			char buffer[256];
			zmq_recv(socket, buffer, 255, 0);
			if (!strcmp(buffer, "Client disconnected")) break;
			else if (!strcmp(buffer, "Client not disconnected")){
				keep_running = 1;
				printf("%s. Try again",buffer);
			}
        }
    }
    
    pthread_join(display_thread, NULL);

    endwin();
    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}
