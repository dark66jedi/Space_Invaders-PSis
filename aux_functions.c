#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "aux_global.h"

struct client_info* createNode(struct client_info** head, char ch, int pos_x, int pos_y, int client_id) {
    struct client_info* newNode = (struct client_info*)malloc(sizeof(struct client_info));
    if (newNode == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->ch = ch;
    newNode->pos_x = pos_x;
    newNode->pos_y = pos_y;
    newNode->client_id = client_id;
    newNode->next = NULL; // Initially, the next pointer is NULL
    return newNode;
}

void insertAtBeginning(struct client_info** head, char ch, int pos_x, int pos_y, int client_id) {
    struct client_info* newNode = createNode(&* head,ch, pos_x, pos_y, client_id);
    newNode->next = *head; // Point the new node's next to the current head
    *head = newNode;       // Update the head to the new node
}

struct client_info* searchNode(struct client_info* head, int target) {
    struct client_info* current = head; // Start from the head
    while (current != NULL) {    // Traverse until the end of the list
        if (current->client_id == target) {
            return current;      // Return the node if data matches
        }
        current = current->next; // Move to the next node
    }
    return NULL;
}

void printList(struct client_info* head) {
    struct client_info* temp = head;
    while (temp != NULL) {
        printf("%d -> ", temp->client_id);
        temp = temp->next;
    }
    printf("NULL\n");
}
