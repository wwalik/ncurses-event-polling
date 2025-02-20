#include <ncurses.h>

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

#define GAME_CONDITION game_state != QUIT

typedef enum
{
       QUIT,
       RUNNING
} GAME_STATE;
GAME_STATE game_state;

#define QUEUE_CAPACITY 256
typedef struct
{
       size_t front;
       size_t rear;
       size_t n_elements;
       int32_t vec[QUEUE_CAPACITY];
} queue_t;

// -1 the queue is full, 0 the operation was successful
int
enqueue(queue_t *q, int32_t element)
{
       if (q->n_elements == QUEUE_CAPACITY)
               return -1;

       q->vec[q->rear] = element;


       q->rear++;
       q->rear %= QUEUE_CAPACITY;

       q->n_elements++;
       return 0;
}

// -1 the queue is empty, 0 the operation was successful
int
dequeue(queue_t *q, int32_t *value)
{
       // The queue is empty
       if (q->n_elements == 0)
               return -1;

       *value = q->vec[q->front];
       q->front++;
       q->front %= QUEUE_CAPACITY;

       q->n_elements--; // TODO
       return 0;
}

static pthread_mutex_t input_queue_lock;
static queue_t input_queue;
void *
handle_input(void *_arg)
{
       int ch;
       while(GAME_CONDITION)
       {
               ch = getch();

               pthread_mutex_lock(&input_queue_lock);
               enqueue(&input_queue, ch);
               pthread_mutex_unlock(&input_queue_lock);
       }

       pthread_mutex_destroy(&input_queue_lock);
       return NULL;
}
int
poll_input(int *ch_ptr)
{
       pthread_mutex_lock(&input_queue_lock);
       int res = dequeue(&input_queue, ch_ptr);
       pthread_mutex_unlock(&input_queue_lock);
       return res;
}



int
main(int argc, char *argv[])
{
       if (argc != 1)
               exit(EXIT_FAILURE);

       // Initialize ncurses
       initscr();
       noecho();
       cbreak();
       curs_set(0); // hide cursor


       // Initialize event queue
       input_queue = (queue_t){ .front = 0, .rear = 0, .n_elements = 0 };
       pthread_mutex_init(&input_queue_lock, NULL);

       // Initialize the input handler
       pthread_t input_thread;
       pthread_create(&input_thread, NULL, handle_input, NULL);

       // Game loop
       game_state = RUNNING;
       int ch;
       while(GAME_CONDITION)
       {
               while (poll_input(&ch) == 0)
               {
                       addch(ch);
               }
               refresh();
       }

       endwin();
       exit(EXIT_SUCCESS);
}
