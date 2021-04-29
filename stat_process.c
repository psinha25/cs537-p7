#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "helper.h"

int main(int argc, char *argv[])
{
    // Check correct number of arguments
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <shm_name> <sleeptime_ms> <num_threads>\n", argv[0]);
        exit(1);
    }

    // Get shared memory name to read from
    char *shm_name = strdup(argv[1]);
    if (shm_name == NULL)
    {
        fprintf(stderr, "shm_name is NULL\n");
        exit(1);
    }

    // Get milliseconds to sleep for
    int sleeptime_ms = atoi(argv[2]);
    if (sleeptime_ms <= 0)
    {
        fprintf(stderr, "sleeptime_ms must be greater than 0\n");
        exit(1);
    }

    // Get number of threads
    int num_threads = atoi(argv[3]);
    if (num_threads <= 0)
    {
        fprintf(stderr, "num_threads must be greater than 0\n");
        exit(1);
    }

    // Open shared memory
    int shmfd = shm_open(shm_name, O_RDWR, 0660);
    if (shmfd < 0)
    {
        fprintf(stderr, "shm_open() failed\n");
        exit(1);
    }

    int pagesize = getpagesize();

    // Map shared memory
    slot_t *shm_ptr = (slot_t *)mmap(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        fprintf(stderr, "mmap() failed\n");
        exit(1);
    }

    // Populate timespec struct for nanosleep() system call
    struct timespec sleeptime;
    sleeptime.tv_sec = sleeptime_ms / 100;
    sleeptime.tv_nsec = (sleeptime_ms % 1000) * 1000000;

    int counter = 1;
    while (1)
    {
        nanosleep(&sleeptime, NULL);
        printf("\n%d", counter);
        for (int i = 0; i < num_threads; i++)
        {
            printf("\n%lu : %d %d %d",
                   shm_ptr[i].TID,
                   shm_ptr[i].requests,
                   shm_ptr[i].static_req,
                   shm_ptr[i].dynamic_req);
        }
        counter++;
    }
}