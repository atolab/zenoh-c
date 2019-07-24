#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"
#include "zenoh/mvar.h"

typedef struct {
   char *name;
   unsigned char *data;
   size_t length;
} resource;

resource sub_last_res;
z_mvar_t *sub_mvar = 0;

resource sto_last_res;
z_mvar_t *sto_mvar = 0;

resource rep_last_res;
z_mvar_t *rep_mvar = 0;

void sub_listener(const z_resource_id_t *rid, const unsigned char *data, size_t length, const z_data_info_t *info, void *arg) {    
  Z_UNUSED_ARG_2(info, arg);
  sub_last_res.name = strdup(rid->id.rname);
  sub_last_res.data = malloc(length);
  memcpy(sub_last_res.data, data, length);
  sub_last_res.length = length;
  z_mvar_put(sub_mvar, &sub_last_res);
}

void sto_listener(const z_resource_id_t *rid, const unsigned char *data, size_t length, const z_data_info_t *info, void *arg) {    
  Z_UNUSED_ARG_2(info, arg);
  sto_last_res.name = strdup(rid->id.rname);
  sto_last_res.data = malloc(length);
  memcpy(sto_last_res.data, data, length);
  sto_last_res.length = length;
  z_mvar_put(sto_mvar, &sto_last_res);
}

void sto_handler(const char *rname, const char *predicate, z_array_resource_t *replies, void *arg) {
  Z_UNUSED_ARG_3(rname, predicate, arg);
  replies->length = 1;
  replies->elem = (z_resource_t**)malloc(sizeof(z_resource_t *));
  replies->elem[0] = (z_resource_t *)malloc(sizeof(z_resource_t));
  replies->elem[0]->rname = sto_last_res.name;
  replies->elem[0]->data = sto_last_res.data;
  replies->elem[0]->length = sto_last_res.length;
  replies->elem[0]->encoding = 0;
  replies->elem[0]->kind = 0;
}

void replies_cleaner(z_array_resource_t *replies, void *arg)
{
  Z_UNUSED_ARG(arg);
  free(replies->elem[0]);
  free(replies->elem);
}

void reply_handler(const z_reply_value_t *reply, void *arg) {
  Z_UNUSED_ARG(arg);
  switch (reply->kind) {
    case Z_STORAGE_DATA: 
      rep_last_res.name = strdup(reply->rname);
      rep_last_res.data = malloc(reply->data_length);
      memcpy(rep_last_res.data, reply->data, reply->data_length);
      rep_last_res.length = reply->data_length;
      break;
    case Z_STORAGE_FINAL:
      break;
    case Z_REPLY_FINAL:
      z_mvar_put(rep_mvar, &rep_last_res);
      break;
  }
}

int main(int argc, char **argv) {
  Z_UNUSED_ARG_2(argc, argv);

  sub_mvar = z_mvar_empty();
  sto_mvar = z_mvar_empty();
  rep_mvar = z_mvar_empty();

  char *locator = strdup("tcp/127.0.0.1:7447");

  z_zenoh_p_result_t z1_r = z_open(locator, 0, 0);
  ASSERT_RESULT(z1_r, "Unable to open session with broker")
  z_zenoh_t *z1 = z1_r.value.zenoh;
  z_start_recv_loop(z1);

  z_pub_p_result_t z1_pub1_r = z_declare_publisher(z1, "/test/large_data/big");
  ASSERT_P_RESULT(z1_pub1_r, "Unable to declare publisher\n");
  z_pub_t *z1_pub1 = z1_pub1_r.value.pub;

  z_zenoh_p_result_t z2_r = z_open(locator, 0, 0);
  ASSERT_RESULT(z2_r, "Unable to open session with broker")
  z_zenoh_t *z2 = z2_r.value.zenoh;
  z_start_recv_loop(z2);

  z_sub_mode_t sm;
  sm.kind = Z_PUSH_MODE;
  z_sub_p_result_t z2_sub1_r = z_declare_subscriber(z2, "/test/large_data/big", &sm, sub_listener, NULL);
  ASSERT_P_RESULT(z2_sub1_r,"Unable to declare subscriber\n");
  z_sub_t *z2_sub1 = z2_sub1_r.value.sub;

  z_sto_p_result_t z2_sto1_r = z_declare_storage(z2, "/test/large_data/big", sto_listener, sto_handler, replies_cleaner, NULL);
  ASSERT_P_RESULT(z2_sto1_r,"Unable to declare storage\n");
  z_sto_t *z2_sto1 = z2_sto1_r.value.sto;

  sleep(1);
  
  resource  sent_res;
  resource *rcvd_res;

  sent_res.name = "/test/large_data/big";
  sent_res.data = malloc(1000000);
  sent_res.length = 1000000;

  z_stream_data(z1_pub1, sent_res.data, sent_res.length);
  rcvd_res = z_mvar_get(sub_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.length == rcvd_res->length);
  assert(0 == memcmp(sent_res.data, rcvd_res->data, sent_res.length));
  rcvd_res = z_mvar_get(sto_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.length == rcvd_res->length);
  assert(0 == memcmp(sent_res.data, rcvd_res->data, sent_res.length));
  
  z_query(z1, "/test/large_data/big", "", reply_handler, NULL);
  rcvd_res = z_mvar_get(rep_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.length == rcvd_res->length);
  assert(0 == memcmp(sent_res.data, rcvd_res->data, sent_res.length));

  z_write_data(z1, sent_res.name, sent_res.data, sent_res.length);
  rcvd_res = z_mvar_get(sub_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.length == rcvd_res->length);
  assert(0 == memcmp(sent_res.data, rcvd_res->data, sent_res.length));
  rcvd_res = z_mvar_get(sto_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.length == rcvd_res->length);
  assert(0 == memcmp(sent_res.data, rcvd_res->data, sent_res.length));

  int i;
  for(i = 64000; i < 67000; ++i) {
    sent_res.length = i;
    z_write_data(z1, sent_res.name, sent_res.data, sent_res.length);
    rcvd_res = z_mvar_get(sub_mvar);
    assert(0 == strcmp(sent_res.name, rcvd_res->name));
    assert(sent_res.length == rcvd_res->length);
    assert(0 == memcmp(sent_res.data, rcvd_res->data, sent_res.length));
    rcvd_res = z_mvar_get(sto_mvar);
    assert(0 == strcmp(sent_res.name, rcvd_res->name));
    assert(sent_res.length == rcvd_res->length);
    assert(0 == memcmp(sent_res.data, rcvd_res->data, sent_res.length));
  }
  for(i = 67000; i < 64000; --i) {
    sent_res.length = i;
    z_write_data(z1, sent_res.name, sent_res.data, sent_res.length);
    rcvd_res = z_mvar_get(sub_mvar);
    assert(0 == strcmp(sent_res.name, rcvd_res->name));
    assert(sent_res.length == rcvd_res->length);
    assert(0 == memcmp(sent_res.data, rcvd_res->data, sent_res.length));
    rcvd_res = z_mvar_get(sto_mvar);
    assert(0 == strcmp(sent_res.name, rcvd_res->name));
    assert(sent_res.length == rcvd_res->length);
    assert(0 == memcmp(sent_res.data, rcvd_res->data, sent_res.length));
  }

  z_undeclare_publisher(z1_pub1);
  z_undeclare_subscriber(z2_sub1);
  z_undeclare_storage(z2_sto1);

  z_close(z1);
  z_close(z2);

  z_stop_recv_loop(z1);
  z_stop_recv_loop(z2);
}
