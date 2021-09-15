
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

unsigned int emergency_up_cnt, emergency_down_cnt, up_cnt, down_cnt, curr_cnt, total_emergency_cnt;
struct semaphore emergency_down_semaphore, wait_up, wait_down, full;
struct semaphore mutex, allPriorityDone;

// direc-> 0=up, 1=down
void ArriveBridge(unsigned int direc, unsigned int prio)
{
    intr_disable();
    if (emergency_up_cnt == 0)
    {
        emergency_down_semaphore.value = emergency_down_cnt;
    }
    if (emergency_down_cnt == 0)
    {
        wait_up.value = up_cnt;
    }
    if (up_cnt == 0)
    {
        wait_down.value = down_cnt;
    }
    if (direc == 0 && prio == 1)
    {
        sema_try_down(&wait_up);
        sema_try_down(&full);
        printf("Emergency vehicle entered from up\n");
    }
    if (direc == 1 && prio == 1)
    {
        sema_try_down(&wait_down);
        sema_try_down(&emergency_down_semaphore);
        sema_try_down(&full);
        printf("Emergency vehicle entered from down\n");
    }
    if (direc == 0 && prio == 0)
    {

        sema_try_down(&wait_up);
        sema_try_down(&full);
        printf("Vehicle entered from up\n");
    }
    if (direc == 1 && prio == 0)
    {

        sema_try_down(&wait_down);
        sema_try_down(&full);
        printf("Vehicle entered from down\n");
    }
}

void CrossBridge(unsigned int direc, unsigned int prio)
{
    intr_enable();
    timer_sleep(2);
    intr_disable();
    printf("\nThread direction %u prio %u crossing \n", direc, prio);
}
void LeaveBridge(unsigned int direc, unsigned int prio)
{
    intr_disable();
    if (direc == 0 && prio == 1)
    {
        emergency_up_cnt--;
        sema_up(&full);
        printf("up emergency thread just Left(direction 0 prio 1)\n");
    }
    if (direc == 1 && prio == 1)
    {
        emergency_down_cnt--;
        sema_up(&full);

        sema_up(&emergency_down_semaphore);
        printf("down emergency thread just Left (direction 1 prio 1)\n");
    }
    if (direc == 0 && prio == 0)
    {
        up_cnt--;
        sema_up(&full);

        sema_up(&wait_up);
        printf("up normal thread just Left(direction 0 prio 0)\n");
    }
    if (direc == 1 && prio == 0)
    {
        down_cnt--;
        sema_up(&full);

        sema_up(&wait_down);
        printf("down normal thread just Left (direction 1 prio 0)\n");
    }
}

void OneVehicle(unsigned int direc, unsigned int prio)
{
    ArriveBridge(direc, prio);
    CrossBridge(direc, prio);
    LeaveBridge(direc, prio);
}
static void emergency_up(void *aux)
{
    OneVehicle(0, 1);
}
static void normal_up(void *aux)
{
    OneVehicle(0, 0);
}

static void emergency_down(void *aux)
{
    OneVehicle(1, 1);
}

static void normal_down(void *aux)
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

    sema_init(&emergency_down_semaphore, 0);
    sema_init(&wait_up, 0);
    sema_init(&wait_down, 0);
    sema_init(&full, 3);

    for (i = 0; i < num_emergency_left; i++){

        const char name = "emergency Up thread " + (char)i;
        thread_create(&name, PRI_DEFAULT, &emergency_up, NULL);
        printf("Created thread dir 0 prio 1\n");
    }
    //  printf("\n Semaphore values; waitemright %u, wait left %u,")
    for (i = 1; i <= num_emergency_right; i++){

        const char name = "emergency down thread " + (char)i;
        thread_create(&name, PRI_DEFAULT, &emergency_down, NULL);
        printf("Created thread dir 1 prio 1\n");
    }
    for (i = 1; i <= up_cnt; i++){

        const char name = "normal up thread " + (char)i;
        thread_create(&name, PRI_DEFAULT, &normal_up, NULL);
        printf("Created thread dir 0 prio 0\n");
    }
    for (i = 1; i <= down_cnt; i++){

        const char name = "normal down thread " + (char)i;
        thread_create(&name, PRI_DEFAULT, &normal_down, NULL);
        printf("Created thread dir 1 prio 0\n");
    }
    intr_disable();
}
