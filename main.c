/* This keyboard event queue could easily be replaced with nodelay(stdscr, TRUE)
 * If you're considering using this code, check the documentation for that function first */
#include <ncurses.h>

#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

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
       size_t n_elements; // Used to check if the queue is empty/full, there's a better way to do this.
       int32_t vec[QUEUE_CAPACITY];
} queue_t;

// Returns -1 if the queue is full; 0 if the operation was successful
int
enqueue(queue_t *q, int32_t element)
{
	/* Alternatively, instead of keeping track of the number of elements,
	 * we could calculate the distance from the front to the rear of the queue.
	 * If this distance is equal to the queue capacity, then the queue is full */
	
	// Is the queue full?
       if (q->n_elements >= QUEUE_CAPACITY)
               return -1;

       q->vec[q->rear] = element;

       q->rear++;
       q->rear %= QUEUE_CAPACITY;

       q->n_elements++;
       return 0;
}
// Returns -1 if the queue is empty; 0 if the operation was successful
int
dequeue(queue_t *q, int32_t *value)
{
	/* Alternatively (q->front == q->rear) */

	// Is the queue empty?
	if (q->n_elements == 0)
		return -1;

       *value = q->vec[q->front];
       q->front++;
       q->front %= QUEUE_CAPACITY;

       q->n_elements--;
       return 0;
}

static queue_t input_queue;
static pthread_mutex_t input_queue_lock;
void *
handle_input(void *_arg)
{
	pthread_mutex_init(&input_queue_lock, NULL);

	int ch;
	while(GAME_CONDITION)
	{
		ch = getch();

		pthread_mutex_lock(&input_queue_lock);

		int res = enqueue(&input_queue, ch);
		if (res != 0)
			// The event queue is full. How did this happen?)
			raise(SIGTRAP); 

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
	{
		printf("Arguments are not expected\n");
		exit(EXIT_FAILURE);
	}

	// Initializes ncurses
	initscr();
	noecho();
	cbreak();
	curs_set(0); // hide cursor

	// Initializes event queue
	input_queue = (queue_t){ .front = 0, .rear = 0, .n_elements = 0 };

	// Initializes the input handler thread
	pthread_t input_thread;
	pthread_create(&input_thread, NULL, handle_input, NULL);

	// Main game loop
	game_state = RUNNING;
	int ch;
	while(GAME_CONDITION)
	{
		// Polls all events
		while (poll_input(&ch) == 0)
		{
			addch(ch);
		}

		refresh();
	}

	endwin();
	// Lets the input thread free its ressources before exiting
	pthread_join(input_thread, NULL);
	exit(EXIT_SUCCESS);
}
