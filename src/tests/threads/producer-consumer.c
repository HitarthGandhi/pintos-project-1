/* Tests producer/consumer communication with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

int producer_cnt, consumer_cnt;

int index=0;
int len=0;
char buffer[10];
char string[] = "Hello World";

struct lock l;
struct condition isNotEmpty, isNotFull;

void producer_consumer(unsigned int num_producer, unsigned int num_consumer);


void test_producer_consumer(void)
{
    /*producer_consumer(0, 0);
    producer_consumer(1, 0);
    producer_consumer(0, 1);
    producer_consumer(1, 1);
    producer_consumer(3, 1);
    producer_consumer(1, 3);
    producer_consumer(4, 4);
    producer_consumer(7, 2);
    producer_consumer(2, 7);*/
    producer_consumer(6, 6);
    pass();
}

void producer(void* num_producer){
    lock_acquire(&l);
    while(index>=10){
        cond_wait(&isNotFull, &l);
    }
    buffer[len]=string[len];
    index++;
    len++;
    cond_signal(&isNotEmpty, &l);
    lock_release(&l);
}

void consumer(void* num_consumer){
    lock_acquire(&l);
    while (index<=0){
        cond_wait(&isNotEmpty, &l);
    }
    for(int i=0; i<sizeof(string); i++){
        buffer[i]=buffer[i+1];
    }
    for(int i=0; i<sizeof(buffer); i++){
        printf("%c",buffer[i]);
        index--;
        cond_signal(&isNotFull, &l);
        lock_release(&l);
    }
}

void producer_consumer(UNUSED unsigned int num_producer, UNUSED unsigned int num_consumer)
{
    // msg("NOT IMPLEMENTED");
    /* FIXME implement */
    producer_cnt=num_producer;
    consumer_cnt=num_consumer;
    lock_init(&l);
    cond_init(&isNotEmpty);
    cond_init(&isNotFull);
    for(int i=0; i<producer_cnt; i++){
        char producers[16];
        thread_create(producers, PRI_DEFAULT, producer, &i);
    }
    for(int i=0; i<consumer_cnt; i++){
        char consumers[16];
        thread_create(consumers, PRI_DEFAULT, consumer, &i);
    }
}


