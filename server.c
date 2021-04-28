//
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

#include "helper.h"
#include "request.h"

// Global buffer userd by one producer and many consumers
int *buffer;

// Lock used with the condition variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Two condition variables: full and empty
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

// Number of full and number of emtpy slots in buffer
int numempty = 0;
int numfull = 0;

// Fill and use pointers in buffer
int fill_ptr = 0;
int use_ptr = 0;

// Size of buffer
int size = 0;

// CS537: Parse the new arguments too
void getargs(int *port, int *numthreads, int *bufsize, int argc, char *argv[])
{
  if (argc != 5)
  {
    fprintf(stderr, "Usage: %s <port> <threads> <buffers> <shm_name>\n", argv[0]);
    exit(1);
  }

  // Port number
  *port = atoi(argv[1]);
  if (*port <= 0 || *port == 22)
    exit(1);

  // Number of user threads
  *numthreads = atoi(argv[2]);
  if (*numthreads <= 0)
    exit(1);

  *bufsize = atoi(argv[3]);
  if (*bufsize <= 0)
    exit(1);
}

// Get the data at the next spot to read from
int getfilled()
{
  int connfd = buffer[use_ptr];
  use_ptr = (use_ptr + 1) % size;
  return connfd;
}

// Handle and close a new request connection
void handle(int connfd)
{
  requestHandle(connfd);
  printf("requestHandle() returned!\n");
  close(connfd);
  printf("close() returned!\n");
}

// Fill the buffer with the specified connfd
void fillbuffer(int connfd)
{
  buffer[fill_ptr] = connfd;
  fill_ptr = (fill_ptr + 1) % size;
  numfull++;
}

// Worker function
void *consumer(void *arg)
{
  while (1)
  {
    pthread_mutex_lock(&mutex);
    while (numfull == 0)
      pthread_cond_wait(&full, &mutex);
    int connfd = getfilled();
    pthread_mutex_unlock(&mutex);
    handle(connfd);
    pthread_mutex_lock(&mutex);
    numempty++;
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
  }
}

int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  int numthreads;
  int bufsize;
  struct sockaddr_in clientaddr;

  getargs(&port, &numthreads, &bufsize, argc, argv);

  printf("Port: %d, NumThreads: %d, BufSize: %d\n", port, numthreads, bufsize);

  // Create a buffer of specified size
  buffer = malloc(sizeof(int) * bufsize);
  numempty = bufsize;
  size = bufsize;

  // Create the specified number of workers
  pthread_t workers[numthreads];
  for (int i = 0; i < numthreads; i++)
  {
    pthread_create(&workers[i], NULL, consumer, NULL);
  }
  printf("Created the specified number of workers!\n");

  for (int i = 0; i < numthreads; ++i)
  {
    pthread_join(workers[i], NULL);
  }
  printf("Joined all the workers!\n");

  //
  // CS537 (Part B): Create & initialize the shared memory region...
  //

  // Producer functionality
  listenfd = Open_listenfd(port);
  printf("Open_listenfd() seemed to work!\n");
  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
    printf("Accept() returned, seemed to work!\n");
    pthread_mutex_lock(&mutex);
    while (numempty == 0)
      pthread_cond_wait(&empty, &mutex);
    fillbuffer(connfd);
    pthread_cond_signal(&full);
    pthread_mutex_unlock(&mutex);
  }
}
