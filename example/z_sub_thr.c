#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"
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

void listener(uint8_t mid, z_resource_id_t rid, z_iobuf_t data) {      
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
  z_zenoh_result_t r_z = z_open(locator, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  zenoh_t z = r_z.value.zenoh;

  z_start_recv_loop(&z);
  printf("Declaring Resource...\n");
  z_vle_result_t r_rid = z_declare_resource(&z, "/home1");
  ASSERT_RESULT(r_rid, "Unable to register result")
  z_vle_t rid = r_rid.value.vle;

  printf("Declaring Subscriber...\n");
  z_sub_mode_t sm;
  sm.kind = Z_PUSH_MODE;
  if (z_declare_subscriber(&z, rid, sm, listener) != 0) {
    printf("Unable to declare pub\n");
    return -1;
  }
  sleep(60);
  return 0;
}