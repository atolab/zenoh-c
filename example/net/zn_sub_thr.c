#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "zenoh.h"

#define N 100000

volatile unsigned long long int count = 0;
volatile struct timeval start;
volatile struct timeval stop;

void print_stats(volatile struct timeval *start, volatile struct timeval *stop) {
  double t0 = start->tv_sec + ((double)start->tv_usec / 1000000.0);
  double t1 = stop->tv_sec + ((double)stop->tv_usec / 1000000.0);
  double thpt = N / (t1 - t0);
  printf("%f msgs/sec\n", thpt);
}

void data_handler(const zn_resource_key_t *rid, const unsigned char *data, size_t length, const zn_data_info_t *info, void *arg) {      
  Z_UNUSED_ARG_5(rid, data, length, info, arg);
  
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
  char *locator = 0;
  if (argc > 1) {
    locator = argv[1];
  }

  zn_session_p_result_t r_z = zn_open(locator, 0, 0);
  ASSERT_RESULT(r_z, "Unable to open session.\n")
  zn_session_t *z = r_z.value.session;
  zn_start_recv_loop(z);  

  zn_sub_mode_t sm;
  sm.kind = ZN_PUSH_MODE;
  zn_sub_p_result_t r = zn_declare_subscriber(z, "/test/thr", &sm, data_handler, NULL);
  ASSERT_P_RESULT(r, "Unable to declare subscriber.\n");
  zn_sub_t *sub = r.value.sub;
  
  sleep(60);

  zn_undeclare_subscriber(sub);
  zn_close(z);
  zn_stop_recv_loop(z);
  return 0;
}
