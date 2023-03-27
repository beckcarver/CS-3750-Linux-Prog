// Beckham Carver
//
// "poor mans generic queue" from problme 7.8
// of Programming Language Pragmatics
// 3-26-23


#include <stdlib.h>
#include <stdio.h>

typedef struct node {
	void* data;
	struct node* next;
} Node;

typedef struct queue {
	Node* head;
	Node* tail;
} Queue;

void enqueue(Queue* queue, void* data) {
	Node *new_node = (Node*)malloc(sizeof(Node));
	new_node->data = data;
	new_node->next = NULL;
	
	if(queue->tail != NULL) {
		queue->tail->next = new_node;
		queue->tail = new_node;
	}
	else {
		queue->head = new_node;
		queue->tail = new_node;
	}
}

void* dequeue(Queue* queue) {
	if(queue->head == NULL) {
		return NULL;
	}
	void* ret = queue->head->data;

	if(queue->head == queue->tail) {
		queue->head == NULL;
		queue->tail == NULL;
	
		}
	else {
		Node* tmp = queue->head;
		queue->head = queue->head->next;
		free(tmp);
	}

	return ret;
}

int main() {
    Queue queue;
    queue.head = NULL;
    queue.tail = NULL;

    int num1 = 1;
    int num2 = 2;
    char *str1 = "hello";
    char *str2 = "world";

    enqueue(&queue, &num1);
    enqueue(&queue, &num2);
    enqueue(&queue, &str1);
    enqueue(&queue, &str2);

    int *p1 = (int*)dequeue(&queue);
    int *p2 = (int*)dequeue(&queue);
    char **p3 = (char**)dequeue(&queue);
    char **p4 = (char**)dequeue(&queue);

    printf("%d\n", *p1);
    printf("%d\n", *p2);
    printf("%s\n", *p3);
    printf("%s\n", *p4);

    return 0;
}




