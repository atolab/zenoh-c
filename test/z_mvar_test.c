#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "zenoh/mvar.h"

char msg[256];

void *sleep_and_fill(void *m) {
  
  z_mvar_t *mv = (z_mvar_t*)m;
  for (int i = 0; i < 10; ++i) {      
    usleep(250000);
    printf("Producing round #%d\n", i);
    sprintf(msg, "Goodie #%d", i); 
    z_mvar_put(mv, msg);
  }
  return 0;
}

void *sleep_and_consume(void *m) {
  z_mvar_t *mv = (z_mvar_t*)m;
  for (int i = 0; i < 10; ++i) {  
    printf("Consuming round #%d\n", i);
    usleep(250000);
    char *msg = (char *)z_mvar_get(mv);
    printf("consumed: MVar content: %s\n", msg);
  }
  return 0;
}
int main(int argc, char **argv) {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  z_mvar_t *mv = z_mvar_empty();
  pthread_t producer;
  pthread_t consumer;
  pthread_create(&producer, 0, sleep_and_fill, mv);    
  pthread_create(&consumer, 0, sleep_and_consume, mv);    
  pthread_join(consumer, NULL);
  pthread_join(producer, NULL);
  
  
  return 0;
}

