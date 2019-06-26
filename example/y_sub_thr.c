#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"
#include "yaks.h"
#include <sys/time.h>


#define N 50000

volatile unsigned long long int count = 0;
volatile struct timeval start;
volatile struct timeval stop;

void print_stats(volatile struct timeval *start, volatile struct timeval *stop) {
  double t0 = start->tv_sec + ((double)start->tv_usec / 1000000.0);
  double t1 = stop->tv_sec + ((double)stop->tv_usec / 1000000.0);
  double thpt = N / (t1 - t0);
  printf("%f msgs/sec\n", thpt);
}

void listener(const z_resource_id_t *rid, const unsigned char *data, size_t length, z_data_info_t info, void *unused) {      
  struct timeval tv;
  if (count == 0) {
    gettimeofday(&tv, 0);
    start = tv;
    count++;
  } else if (count < N) {
    count++;
  } else {
    gettimeofday(&tv, 0);
    stop = tv;
    print_stats(&start, &stop);
    count = 0;
  }    
}

int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_p_result_t r_z = z_open(locator, 0, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;

  z_start_recv_loop(z);
  printf("Declaring Resource...\n");
  y_subscribe(z, "/perf/put/thr", listener, NULL);
  sleep(60000);
  return 0;
}