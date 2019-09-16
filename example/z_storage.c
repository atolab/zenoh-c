#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"
#include "zenoh/rname.h"

#define MAX_LEN 256

typedef struct sample {
  char *rname;
  char *data;
  size_t length;
} sample_t;

z_list_t *stored = 0;

int remove_data(void *elem, void*args){
  sample_t *sample = (sample_t*)elem;
  if(strcmp(sample->rname, (char *)args) == 0){
    free(sample->rname);
    free(sample->data);
    return 1;
  }
  return 0;
}

void listener(const z_resource_id_t *rid, const unsigned char *data, size_t length, const z_data_info_t *info, void *arg) {    
  Z_UNUSED_ARG_2(info, arg);
  char str[MAX_LEN];
  memcpy(&str, data, length < MAX_LEN ? length : MAX_LEN - 1);
  str[length < MAX_LEN ? length : MAX_LEN - 1] = 0;
  printf(">> [Storage listener] Received ('%20s' : '%s')\n", rid->id.rname, str);
  stored = z_list_remove(stored, remove_data, rid->id.rname);

  sample_t *sample = (sample_t *)malloc(sizeof(sample_t));
  sample->rname = strdup(rid->id.rname);
  sample->data = malloc(length);
  memcpy(sample->data, data, length);
  sample->length = length;

  stored = z_list_cons(stored, sample);
}

void query_handler(const char *rname, const char *predicate, replies_sender_t send_replies, void *query_handle, void *arg) {
  Z_UNUSED_ARG(arg);
  printf(">> [Query handler   ] Handling '%s?%s'\n", rname, predicate);
  z_array_resource_t replies;
  z_list_t *matching_samples = 0;

  z_list_t *samples = stored;
  sample_t *sample;
  while (samples != z_list_empty) {
    sample = (sample_t *) z_list_head(samples);
    if(intersect((char *)rname, sample->rname))
    {
      matching_samples = z_list_cons(matching_samples, sample);
    }
    samples = z_list_tail(samples);
  }
  replies.length = z_list_len(matching_samples);

  z_resource_t resources[replies.length];
  z_resource_t *p_resources[replies.length];
  
  samples = matching_samples; 
  int i =0;
  while (samples != z_list_empty) {
    sample = (sample_t *) z_list_head(samples);
    resources[i].rname = sample->rname;
    resources[i].data = (const unsigned char *)sample->data;
    resources[i].length = sample->length;
    resources[i].encoding = 0;
    resources[i].kind = 0;
    p_resources[i] = &resources[i];
    samples = z_list_tail(samples);
    ++i;
  }
  z_list_free(&matching_samples);

  replies.elem = &p_resources[0];

  send_replies(query_handle, replies);
}

int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }
  char *uri = "/demo/example/**";
  if (argc > 2) {
    uri = argv[2];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_p_result_t r_z = z_open(locator, 0, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;
  z_start_recv_loop(z);

  printf("Declaring Storage on '%s'...\n", uri);
  z_sto_p_result_t r = z_declare_storage(z, uri, listener, query_handler, NULL);
  ASSERT_P_RESULT(r, "Unable to declare storage\n");  
  z_sto_t *sto = r.value.sto;

  char c = 0;
  while (c != 'q') {
    c = fgetc(stdin);
  }

  z_undeclare_storage(sto);
  z_close(z);
  z_stop_recv_loop(z);
  return 0;
}
