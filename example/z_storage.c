#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"
#include "zenoh/rname.h"


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

void listener(const z_resource_id_t *rid, const unsigned char *data, size_t length, z_data_info_t *info, void *unused) {    
  printf("Received data: %s\n", rid->id.rname);
  stored = z_list_remove(stored, remove_data, rid->id.rname);

  sample_t *sample = (sample_t *)malloc(sizeof(sample_t));
  sample->rname = strdup(rid->id.rname);
  sample->data = malloc(length);
  memcpy(sample->data, data, length);
  sample->length = length;

  stored = z_list_cons(stored, sample);
}

z_array_z_resource_t query_handler(const char *rname, const char *predicate, void *unused) {
  printf("Handling Query: %s\n", rname);
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
  Z_ARRAY_S_MAKE(z_resource_t, replies, z_list_len(matching_samples))
  samples = matching_samples; 
  int i =0;
  while (samples != z_list_empty) {
    sample = (sample_t *) z_list_head(samples);
    replies.elem[i].rname = sample->rname;
    replies.elem[i].data = (const unsigned char *)sample->data;
    replies.elem[i].length = sample->length;
    replies.elem[i].encoding = 0;
    replies.elem[i].kind = 0;
    samples = z_list_tail(samples);
    ++i;
  }
  z_list_free(&matching_samples);
  return replies;
}

void replies_cleaner(z_array_z_resource_t replies, void *unused)
{
  printf("Cleaning Replies.\n");
  Z_ARRAY_S_FREE(replies)
}


int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }
  char *uri="/demo/**";
  if (argc > 2) {
    uri = argv[2];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_p_result_t r_z = z_open(locator, 0, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;

  z_start_recv_loop(z);
  printf("Declaring Storage: %s\n", uri);
  z_declare_storage(z, uri, listener, query_handler, replies_cleaner, NULL);

  while (1) { 
    sleep(1);
  }
  return 0;
}