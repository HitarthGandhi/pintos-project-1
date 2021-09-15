
/* Tests producer/consumer communication with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */

#include <stdio.h>
#include <stdlib.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

void narrow_bridge(unsigned int num_vehicles_left, unsigned int num_vehicles_right,
                   unsigned int num_emergency_left, unsigned int num_emergency_right);
void ArriveBridge(unsigned int direc, unsigned int prio);           
void CrossBridge(unsigned int direc, unsigned int prio);
void LeaveBridge(unsigned int direc, unsigned int prio);
void OneVehicle(unsigned int direc, unsigned int prio);

unsigned int emergency_up_cnt, emergency_down_cnt, up_cnt, down_cnt, curr_cnt;
struct semaphore emergency_down_semaphore, wait_left, wait_right, full;

// direc-> 0=up, 1=down
void ArriveBridge(unsigned int direc, unsigned int prio)
{
    intr_disable();
    //printf("\n SOmeone arrived\n");
    if (emergency_up_cnt == 0)
    {
        //sema_init(&emergency_down_semaphore,emergency_down_cnt);
        emergency_down_semaphore.value = emergency_down_cnt;
    }
    if (emergency_down_cnt == 0)
    {
        //sema_init(&wait_left,up);
        wait_left.value = up_cnt;
    }
    if (up_cnt == 0)
    {
        //sema_init(&wait_right,down_cnt);
        wait_right.value = down_cnt;
    }
    /*printf("\nSmeaphores: wait em right %u wait left %u wait right %u full %u",
			emergency_down_semaphore.value,wait_left.value,wait_right.value,full.value);*/
    if (direc == 0 && prio == 1)
    {
        sema_try_down(&full);
        // printf("\nLeft em thread entered (direction 0 prio 1)");
    }
    if (direc == 1 && prio == 1)
    {

        sema_try_down(&emergency_down_semaphore);
        sema_try_down(&full);
        // printf("\n Right em thread entered(direction 1 prio 1)");
    }
    if (direc == 0 && prio == 0)
    {

        sema_try_down(&wait_left);
        sema_try_down(&full);
        // printf("\n Left thread entered(direction0 prio 0)");
    }
    if (direc == 1 && prio == 0)
    {

        sema_try_down(&wait_right);
        sema_try_down(&full);
        // printf("\n right thread entered (direction 1 prio 0)");
    }
}

void CrossBridge(unsigned int direc, unsigned int prio)
{
    // intr_enable();
    // timer_sleep(2);
    // intr_disable();
    // printf("\nThread direction %u prio %u crossing \n", direc, prio);
}
void LeaveBridge(unsigned int direc, unsigned int prio)
{
    intr_disable();
    //printf("\n Leaving");
    if (direc == 0 && prio == 1)
    {
        emergency_up_cnt--;
        //full.value++;
        sema_up(&full);
        // printf("Left emergency thread just Left(direction 0 prio 1)");
        //thread_exit();
    }
    if (direc == 1 && prio == 1)
    {
        emergency_down_cnt--;
        sema_up(&full);

        sema_up(&emergency_down_semaphore);
        // printf("right emergency thread just Left (direction 1 prio 1)");
        //thread_exit();
    }
    if (direc == 0 && prio == 0)
    {
        up_cnt--;
        sema_up(&full);

        sema_up(&wait_left);
        // printf("left normal thread just Left(direction 0 prio 0)");
        //	thread_exit();
    }
    if (direc == 1 && prio == 0)
    {
        down_cnt--;
        sema_up(&full);

        sema_up(&wait_right);
        // printf("right normal thread just Left (direction 1 prio 0)\n");
        //thread_exit();
    }
}

void OneVehicle(unsigned int direc, unsigned int prio)
{
    ArriveBridge(direc, prio);
    CrossBridge(direc, prio);
    LeaveBridge(direc, prio);
}
static void left_em(void *aux)
{
    OneVehicle(0, 1);
}
static void left_normal(void *aux)
{
    OneVehicle(0, 0);
}

static void right_em(void *aux)
{
    OneVehicle(1, 1);
}

static void right_normal(void *aux)
{
    OneVehicle(1, 0);
}

void test_narrow_bridge(void)
{
    // narrow_bridge(0, 0, 0, 0);
    // narrow_bridge(1, 0, 0, 0);
    // narrow_bridge(0, 0, 0, 1);
    // narrow_bridge(0, 4, 0, 0);
    // narrow_bridge(0, 0, 4, 0);
    // narrow_bridge(3, 3, 3, 3);
    // narrow_bridge(4, 3, 4 ,3);
    // narrow_bridge(7, 23, 17, 1);
    // narrow_bridge(40, 30, 0, 0);
    // narrow_bridge(30, 40, 0, 0);
    // narrow_bridge(23, 23, 1, 11);
    narrow_bridge(22, 22, 10, 10);
    // narrow_bridge(0, 0, 11, 12);
    // narrow_bridge(0, 10, 0, 10);
    // narrow_bridge(10, 10, 10, 10);
    pass();
}

void narrow_bridge(UNUSED unsigned int num_vehicles_left, UNUSED unsigned int num_vehicles_right,
                   UNUSED unsigned int num_emergency_left, UNUSED unsigned int num_emergency_right){
    // msg("NOT IMPLEMENTED");
    /* FIXME implement */

    unsigned int i;
    emergency_up_cnt = num_emergency_left;
    emergency_down_cnt = num_emergency_right;
    up_cnt = num_vehicles_left;
    down_cnt = num_vehicles_right;

    /* emergency_down_semaphore=(struct semaphore)malloc(sizeof(struct semaphore));
    wait_left=(struct semaphore)malloc(sizeof(struct semaphore));
    wait_right=(struct semaphore)malloc(sizeof(struct semaphore));
    full=(struct semaphore)malloc(sizeof(struct semaphore));*/
    sema_init(&emergency_down_semaphore, 0);
    sema_init(&wait_left, 0);
    sema_init(&wait_right, 0);
    sema_init(&full, 3);
    // full.value=3;
    // printf("\nFull is %u\n",full.value);
    //  printf("\nEM LEFT: %u EM RIGHT: %u Left:%u right:%u",num_emergency_left,num_emergency_right,rm_left,down_cnt);
    for (i = 0; i < num_emergency_left; i++){

        const char name = "THread em left " + (char)i;
        // Prints "Hello world!" on hello_world
        //sprintf(name, "%Thread_em_left %u", i);
        thread_create(&name, PRI_DEFAULT, &left_em, NULL);
        // printf("\nCreated thread dir 0 prio 1");
    }
    //  printf("\n Semaphore values; waitemright %u, wait left %u,")
    for (i = 1; i <= num_emergency_right; i++){

        const char name = "thread em right " + (char)i;
        // Prints "Hello world!" on hello_world
        // sprintf(name, "%Thread_em_right %u", i);
        thread_create(&name, PRI_DEFAULT, &right_em, NULL);
        // printf("\nCreated thread dir 1 prio 1");
    }
    for (i = 1; i <= up_cnt; i++){

        const char name = "thread_n_left " + (char)i;
        //(char*)malloc(20 * sizeof(char));
        // Prints "Hello world!" on hello_world
        // sprintf(name, "%Thread_normal_left %u", i);
        thread_create(&name, PRI_DEFAULT, &left_normal, NULL);
        // printf("\nCreated thread dir 0 prio 0 ");
    }
    for (i = 1; i <= down_cnt; i++){

        const char name = "thread_n_right " + (char)i;
        // Prints "Hello world!" on hello_world
        // sprintf(name, "%Thread_right_normal %u", i);
        thread_create(&name, PRI_DEFAULT, &right_normal, NULL);
        // printf("\nCreated thread dir 1 prio 0");
    }
    intr_disable();
    thread_start();
}
