#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t *q)
{
        if (q == NULL)
                return 1;
        return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
        /* TODO: put a new process to queue [q] */
        //Check if the queue is full before adding
        if (q->size < MAX_QUEUE_SIZE) {
                q->proc[q->size] = proc;
                q->size++;
        }
        else {
                fprintf(stderr, "Queue is full, cannot enqueue process.\n");
        }
}

struct pcb_t *dequeue(struct queue_t *q)
{
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        //If queue is empty, return NULL
        if (q->size == 0) {
                return NULL;
        }

        //Therefore, we take the first process in the list (FIFO).
        struct pcb_t *proc = q->proc[0];

        for (int i = 0; i < q->size - 1; i++) {    //Shift the remaining processes to the left to fill the gap
                q->proc[i] = q->proc[i + 1];
        }

        //Decrease the size of the queue
        q->size--;
        q->proc[q->size] = NULL; //Clear the last pointer 

        return proc;
}

struct pcb_t *purgequeue(struct queue_t *q, struct pcb_t *proc)
{
        /* TODO: remove a specific item from queue
         * */
       if (empty(q)) return NULL;

    int index = -1;
    struct pcb_t *found_proc = NULL;

    //Find the index of the process in the queue
    for (int i = 0; i < q->size; i++) {
        if (q->proc[i] == proc) {
            index = i;
            found_proc = q->proc[i];
            break;
        }
    }

    //If process found, remove it and shift remaining elements
    if (index != -1) {
        for (int i = index; i < q->size - 1; i++) {
            q->proc[i] = q->proc[i + 1];
        }
        q->size--;
        q->proc[q->size] = NULL;
        return found_proc;
    }

    return NULL;
}