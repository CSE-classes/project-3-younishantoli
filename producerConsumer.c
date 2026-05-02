#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define BUFFERSIZE 5

char buffer[BUFFERSIZE]; //the shared queue
int in = 0; //producer's input
int out = 0; //consumer's removals
int count = 0; //number of current items in the buffer
int done = 0; //this is like a flag that tells the consumer that the producer is done

pthread_mutex_t lock; //this locks and like protects the shared data
pthread_cond_t notFull; //like flags buffer is not full
pthread_cond_t notEmpty; //like flags that the buffer isn't empty
FILE *fp; //this is the pointer to the file


void *producer(void *arg) {
	char c;
while (1) {
	c = fgetc(fp); //fetch one character from the file pointer we initialized

	if(c == EOF) {
		pthread_mutex_lock(&lock); //basically a thread at a time here
		done = 1; //flag that the producer is done
		pthread_cond_signal(&notEmpty); //let the consumer know
		pthread_mutex_unlock(&lock); //unlock after changing 
		break;
	}

	pthread_mutex_lock(&lock); //lock b

	while(count == BUFFERSIZE) { //the queue is full
		pthread_cond_wait(&notFull, &lock); //basically like wait until the bufer isn't full anymore
	}

	buffer[in] = c; //put the character in the buffer
	in = (in + 1) % BUFFERSIZE; //go to next spot
	count++; //keep count of how much the buffer has, so increment count

	pthread_cond_signal(&notEmpty); //after putting something into the buffer, it basically calls that it isn't empty anymore
	pthread_mutex_unlock(&lock); //like it says, it unlocks and the thread is done 
}

return NULL;
}

void *consumer(void *arg) {
	char c;

	while(1) {
		pthread_mutex_lock(&lock); //lock before buffer usage

		while(count == 0 && done == 0) {
			pthread_cond_wait(&notEmpty, &lock); //consumer basically sleeps until the producer adds something
		}

		if(count == 0 && done == 1) { 
			pthread_mutex_unlock(&lock); //unlock before stopping
			break;
		}

		c = buffer[out]; //take one character from the buffer
		out = (out + 1) % BUFFERSIZE; //next spot
		count--; //buffer count decreases

		pthread_cond_signal(&notFull); //lets the producer know that there's space; not full.
		pthread_mutex_unlock(&lock); //unlock buffer

		printf("%c", c);
	}

	return NULL;
}

int main() {
	pthread_t producerThread;
	pthread_t consumerThread;

	fp = fopen("message.txt", "r"); //open our messages.txt file

	pthread_mutex_init(&lock, NULL); //create mutex
	pthread_cond_init(&notFull, NULL);
	pthread_cond_init(&notEmpty, NULL);

	pthread_create(&producerThread, NULL, producer, NULL);
	pthread_create(&consumerThread, NULL, consumer, NULL);

	pthread_join(producerThread, NULL); //wait until producer is done
	pthread_join(consumerThread, NULL); //wait until consumer is done

	return 0;
}
