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
    int sleeptime = atoi(argv[2]);
    int num_threads = atoi(argv[3]);

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

    printf("shm_name: %s, sleeptime_ms: %i, num_threads: %i", shm_name, sleeptime, num_threads);
}