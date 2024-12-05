typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

typedef struct remote_char_t
{   
    int msg_type; /* 0 join   1 - move */
    char client_id[16]; 
    direction_t direction ;
}remote_char_t;

#define FIFO_NAME "/tmp/fifo_snail"
