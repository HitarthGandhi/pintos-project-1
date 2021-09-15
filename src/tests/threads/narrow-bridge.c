/* Tests producer/consumer communication with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */

#include <stdio.h>
#include <list.h>
#include "devices/timer.h"
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "lib/random.h"

#define MAX_VEHICLES_ON_BRIDGE 3

/* Structure for Vehicle */
struct Vehicle
{
    char *name;            /* Vehicle name */
    int direc;         /* Vehicle direction */
    int prio;          /* Vehicle priority */
    struct semaphore sema; /* Semaphore for this vehicle */
    struct list_elem elem; /* To store it in a list */
} typedef Vehicle;

void narrow_bridge(unsigned int num_vehicles_left, unsigned int num_vehicles_right,
                   unsigned int num_emergency_left, unsigned int num_emergency_right);
void One_Vehicle(void *args);
void Arrive_Bridge(Vehicle *vehicle);
void Cross_Bridge(Vehicle *vehicle);
void Leave_Bridge(Vehicle *vehicle);
void schedule();

struct list emergency_list;  /* List of waiting emergency vehicles */
struct list normal_list;     /* List of waiting normal vehicles */
struct semaphore mutex_lock; /* Lock to access shared variables */

int no_of_vehicles, cur_direction; /* Current number of vehicles crossing the bridge and their direction */

void test_narrow_bridge(void)
{
    /*narrow_bridge(0, 0, 0, 0);
    narrow_bridge(1, 0, 0, 0);
    narrow_bridge(0, 0, 0, 1);
    narrow_bridge(0, 4, 0, 0);
    narrow_bridge(0, 0, 4, 0);
    narrow_bridge(3, 3, 3, 3);
    narrow_bridge(4, 3, 4 ,3);
    narrow_bridge(7, 23, 17, 1);
    narrow_bridge(40, 30, 0, 0);
    narrow_bridge(30, 40, 0, 0);
    narrow_bridge(23, 23, 1, 11);
    narrow_bridge(22, 22, 10, 10);
    narrow_bridge(0, 0, 11, 12);
    narrow_bridge(0, 10, 0, 10);*/
    narrow_bridge(0, 10, 10, 0);
    pass();
}

void narrow_bridge(UNUSED unsigned int num_vehicles_left, UNUSED unsigned int num_vehicles_right,
                   UNUSED unsigned int num_emergency_left, UNUSED unsigned int num_emergency_right)
{
    /* Initializes shared variables and semaphore */
    list_init(&emergency_list);
    list_init(&normal_list);
    sema_init(&mutex_lock, 1); /* Initialize with value 1 */
    no_of_vehicles = 0;
    cur_direction = -1;
    random_init((unsigned int)12345678);

    /* Create threads for left direc 0 prio vehicles */
    for (int i = 0; i < num_vehicles_left; i++)
    {
        Vehicle *left_vehicle;
        left_vehicle = (Vehicle *)malloc(sizeof(Vehicle));
        struct semaphore sema_vehicle;
        left_vehicle->name = "NL";
        left_vehicle->direc = 0;
        left_vehicle->prio = 0;
        left_vehicle->sema = sema_vehicle;
        sema_init(&(left_vehicle->sema), 0); /* Initialize sema_vehicle with 0 */
        thread_create("NL", PRI_DEFAULT, One_Vehicle, left_vehicle);
    }

    /* Create threads for right direc 0 prio vehicles */
    for (int i = 0; i < num_vehicles_right; i++)
    {
        Vehicle *right_vehicle;
        right_vehicle = (Vehicle *)malloc(sizeof(Vehicle));
        struct semaphore sema_vehicle;
        right_vehicle->name = "NR";
        right_vehicle->direc = 1;
        right_vehicle->prio = 0;
        right_vehicle->sema = sema_vehicle;
        sema_init(&(right_vehicle->sema), 0); /* Initialize sema_vehicle with 0 */
        thread_create("NR", PRI_DEFAULT, One_Vehicle, right_vehicle);
    }

    /* Create threads for left direc 1 prio vehicles */
    for (int i = 0; i < num_emergency_left; i++)
    {
        Vehicle *left_vehicle;
        left_vehicle = (Vehicle *)malloc(sizeof(Vehicle));
        struct semaphore sema_vehicle;
        left_vehicle->name = "EL";
        left_vehicle->direc = 0;
        left_vehicle->prio = 1;
        left_vehicle->sema = sema_vehicle;
        sema_init(&(left_vehicle->sema), 0); /* Initialize sema_vehicle with 0 */
        thread_create("EL", PRI_DEFAULT, One_Vehicle, left_vehicle);
    }

    /* Create threads for right direc 1 prio vehicles */
    for (int i = 0; i < num_emergency_right; i++)
    {
        Vehicle *right_vehicle;
        right_vehicle = (Vehicle *)malloc(sizeof(Vehicle));
        struct semaphore sema_vehicle;
        right_vehicle->name = "ER";
        right_vehicle->direc = 1;
        right_vehicle->prio = 1;
        right_vehicle->sema = sema_vehicle;
        sema_init(&(right_vehicle->sema), 0); /* Initialize sema_vehicle with 0 */
        thread_create("ER", PRI_DEFAULT, One_Vehicle, right_vehicle);
    }

    return;
}

/* One_Vehicle function which is being called by each vehicle thead */
void One_Vehicle(void *args)
{
    Vehicle *vehicle = (Vehicle *)args;
    Arrive_Bridge(vehicle);
    Cross_Bridge(vehicle);
    Leave_Bridge(vehicle);
    return;
}

/* Arrive bridge function */
void Arrive_Bridge(Vehicle *vehicle)
{
    sema_down(&mutex_lock); /* Acquires semaphore to access shared vehicles  */

    /* Append the vehicle to emergency_list if it is of higher priority, else append in normal_list */
    if (vehicle->prio == 1)
    {
        list_push_back(&emergency_list, &(vehicle->elem));
    }
    else
    {
        list_push_back(&normal_list, &(vehicle->elem));
    }

    schedule();                  /* Schedule the vehicle */
    sema_up(&mutex_lock);        /* Releases the lock */
    sema_down(&(vehicle->sema)); /* Wait till it gets signaled */
    return;
}

void Cross_Bridge(Vehicle *vehicle)
{
    int64_t cross_time = (int64_t)random_ulong() % 100; /* Generates random cross_time between 0 and 100 */
    printf("%s started crossing the bridge from %d for time: %lld.\n", vehicle->name, vehicle->direc, cross_time);
    timer_sleep(cross_time); /* Put the current thread to sleep for cross_time ticks */
    return;
}

void Leave_Bridge(Vehicle *vehicle)
{
    sema_down(&mutex_lock); /* Acquire the lock to access shared variables */
    printf("%s crossed the bridge from %d\n", vehicle->name, vehicle->direc);
    no_of_vehicles--; /* Decrement the current number of crossing vehicles */

    /* Update cur_direction to -1 if there is no vehicle crossing the bridge */
    if (no_of_vehicles == 0)
    {
        cur_direction = -1;
    }

    /* Schedule new vehicle (Called three time because at one time maximum three vehicles can be scheduled) */
    for (int i = 0; i < MAX_VEHICLES_ON_BRIDGE; i++)
    {
        schedule();
        schedule();
        schedule();
    }

    sema_up(&mutex_lock); /* Releases the lock */
    return;
}

/* Function to schedule the vehicles (mutex_lock must have been acquired before calling schedule) */
void schedule()
{
    /* Check if currently there are less than 3 vehicles on the road, else return */
    if (no_of_vehicles < MAX_VEHICLES_ON_BRIDGE)
    {
        /* If there is at least one emergency vehicle waiting, else check for waiting normal vehicles */
        if (!list_empty(&emergency_list))
        {
            /* Using FIFO approach */
            /* Extract the maximum waited emergency vehicle */
            struct list_elem *first = list_begin(&emergency_list);
            Vehicle *vehicle = list_entry(first, Vehicle, elem);

            /* If no vehicle is crossing the bridge or crossing in the same direction then signal this emergency vehicle */
            if (cur_direction == -1)
            {
                cur_direction = vehicle->direc; /* Update current direction */
                no_of_vehicles++;                   /* Increment number of vehicles crossing the bridge */
                sema_up(&(vehicle->sema));          /* Signal the vehicle */
                list_remove(first);                 /* Remove the vehicle from waiting list */
            }
            else if (cur_direction == vehicle->direc)
            {
                no_of_vehicles++;          /* Increment number of vehicles crossing the bridge */
                sema_up(&(vehicle->sema)); /* Signal the vehicle */
                list_remove(first);        /* Remove the vehicle from waiting list */
            }
        }
        else if (!list_empty(&normal_list))
        {
            /* Using FIFO approach */
            /* Extract the maximum waited normal vehicle */
            struct list_elem *first = list_begin(&normal_list);
            Vehicle *vehicle = list_entry(first, Vehicle, elem);

            /* If no vehicle is crossing the bridge or crossing in the same direction then signal this normal vehicle */
            if (cur_direction == -1)
            {
                cur_direction = vehicle->direc; /* Update current direction */
                no_of_vehicles++;                   /* Increment number of vehicles crossing the bridge */
                sema_up(&(vehicle->sema));          /* Signal the vehicle */
                list_remove(first);                 /* Remove the vehicle from waiting list */
            }
            else if (cur_direction == vehicle->direc)
            {
                no_of_vehicles++;          /* Increment number of vehicles crossing the bridge */
                sema_up(&(vehicle->sema)); /* Signal the vehicle */
                list_remove(first);        /* Remove the vehicle from waiting list */
            }
        }
    }
    return;
}
