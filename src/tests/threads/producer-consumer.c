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
int id=0, len=0, len1=11; 

char buffer[11];
char string[] = "Hello World";

struct lock l;
struct condition isNotEmpty, isNotFull;

void producer_consumer(unsigned int num_producer, unsigned int num_consumer);
void* producer(void* num_producer);
void* consumer(void* num_consumer);

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

void* producer(void* num_producer){
    while(true){
        lock_acquire(&l);
        if(id==11){
            cond_wait(&isNotFull,&l);
        }
        if(len==11) {
            lock_release(&l);
            break;
        }
        // buffer is not full
        // store the next character in buffer
        buffer[id]=string[id];
        id++;
        len++;
        // set buffer to not empty
        // so that consumer can resume
        cond_signal(&isNotEmpty, &l);
        lock_release(&l);
    }
    return;
}

void* consumer(void* num_consumer){
    while(true){
        lock_acquire(&l);
        if (id==-1){ // buffer is empty
            // so wait for producer to put something
            cond_wait(&isNotEmpty, &l);
        }
        if(len1==0){
            lock_release(&l);
            break;
        }
        char c = buffer[0];
        for(int i=0; i<sizeof(buffer);i++)
            buffer[i]=buffer[i+1];
        // print character
        printf("%c",c);
        id--; len1--;
        cond_signal(&isNotFull, &l);
        lock_release(&l);
    }
    return;
}

void producer_consumer(UNUSED unsigned int num_producer, UNUSED unsigned int num_consumer)
{
    // msg("NOT IMPLEMENTED");
    /* FIXME implement */
    lock_init(&l);
    cond_init(&isNotEmpty);
    cond_init(&isNotFull);
    tid_t pr[num_producer], co[num_consumer];
    int i;
    for(i=0; i<num_producer; i++){
        char producers[16];
        pr[i] = thread_create(producers, i, producer, &i);
    }
    for(i=0; i<num_consumer; i++){
        char consumers[16];
        co[i] = thread_create(consumers, i, consumer, &i);
    }
    return;
}


