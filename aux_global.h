#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct client_info
{
    int ch;
    int pos_x, pos_y;
    unsigned int client_id;
    struct client_info* next;
} client_info;

struct client_info* createNode(struct client_info** head, char ch, int pos_x, int pos_y, int client_id);
void insertAtBeginning(struct client_info** head, char ch, int pos_x, int pos_y, int client_id);
struct client_info* searchNode(struct client_info* head, int target);
void printList(struct client_info* head);