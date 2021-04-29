#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "helper.h"

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <port> <threads> <buffers> <shm_name>\n", argv[0]);
        exit(1);
    }

    char *shm_name = argv[1];
    int sleeptime_ms = atoi(argv[2]);
    if (sleeptime_ms <= 0)
        exit(1);
    int num_threads = atoi(argv[3]);
    if (num_threads <= 0)
        exit(1);

    int shmfd = shm_open(shm_name, O_RDWR, 0660);
    if (shmfd < 0)
    {
        perror("shm_open() failed\n");
        exit(1);
    }

    int pagesize = getpagesize();

    slot_t *shm_ptr = (slot_t *)mmap(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        perror("mmap() failed\n");
        exit(1);
    }

    int sleeptime_s = sleeptime_ms / 100;
    int sleeptime_ns = (sleeptime_ms % 1000) * 1000000;
    struct timespec sleeptime;
    sleeptime.tv_sec = sleeptime_s;
    sleeptime.tv_nsec = sleeptime_ns;
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