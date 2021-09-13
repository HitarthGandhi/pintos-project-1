/* Tests producer/consumer communication with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

// place where new character in buffer is stored
// if it is 10 then buffer is full
int index=0; 

char buffer[10];
char string[] = "Hello World";

struct lock l;
struct condition isNotEmpty, isNotFull;

void producer_consumer(unsigned int num_producer, unsigned int num_consumer);


void test_producer_consumer(void)
{
    producer_consumer(0, 0);
    producer_consumer(1, 0);
    producer_consumer(0, 1);
    producer_consumer(1, 1);
    producer_consumer(3, 1);
    producer_consumer(1, 3);
    producer_consumer(4, 4);
    producer_consumer(7, 2);
    producer_consumer(2, 7);
    producer_consumer(6, 6);
    pass();
}

void producer(void* num_producer){
    lock_acquire(&l);
    while(index>=10){ // buffer is full
        // wait until consumer removes something from it
        cond_wait(&isNotFull, &l);
    }
    // buffer is not full
    // store the next character in buffer
    buffer[index]=string[index];
    index++;
    // set buffer to not empty
    // so that consumer can resume
    cond_signal(&isNotEmpty, &l);
    lock_release(&l);
}

void consumer(void* num_consumer){
    lock_acquire(&l);
    while (index<=0){ // buffer is empty
        // so wait for producer to put something
        cond_wait(&isNotEmpty, &l);
    }
    // print character
    for(int i=0; i<sizeof(buffer); i++){
        printf("%c",buffer[i]);
        index--;
        // set the buffer to not full
        // so that producer can resume
        cond_signal(&isNotFull, &l);
        lock_release(&l);
    }
}

void producer_consumer(UNUSED unsigned int num_producer, UNUSED unsigned int num_consumer)
{
    // msg("NOT IMPLEMENTED");
    /* FIXME implement */
    lock_init(&l);
    cond_init(&isNotEmpty);
    cond_init(&isNotFull);
    for(int i=0; i<num_producer; i++){
        char producers[16];
        thread_create(producers, PRI_DEFAULT, producer, &i);
    }
    for(int i=0; i<num_consumer; i++){
        char consumers[16];
        thread_create(consumers, PRI_DEFAULT, consumer, &i);
    }
}


