#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"
#include "zenoh/mvar.h"

typedef struct {
   char *name;
   unsigned char data;
} resource;

resource z1_sub1_last_res;
z_mvar_t *z1_sub1_mvar = 0;

resource z2_sub1_last_res;
z_mvar_t *z2_sub1_mvar = 0;

resource z1_sto1_last_res;
z_mvar_t *z1_sto1_mvar = 0;

resource z2_sto1_last_res;
z_mvar_t *z2_sto1_mvar = 0;

z_list_t *replies = 0;
z_mvar_t *replies_mvar = 0;

void z1_sub1_listener(const z_resource_id_t *rid, const unsigned char *data, size_t length, const z_data_info_t *info, void *arg) {    
  Z_UNUSED_ARG_3(length, info, arg);
  z1_sub1_last_res.name = strdup(rid->id.rname);
  z1_sub1_last_res.data = *data;
  z_mvar_put(z1_sub1_mvar, &z1_sub1_last_res);
}

void z2_sub1_listener(const z_resource_id_t *rid, const unsigned char *data, size_t length, const z_data_info_t *info, void *arg) {    
  Z_UNUSED_ARG_3(length, info, arg);
  z2_sub1_last_res.name = strdup(rid->id.rname);
  z2_sub1_last_res.data = *data;
  z_mvar_put(z2_sub1_mvar, &z2_sub1_last_res);
}

void z1_sto1_listener(const z_resource_id_t *rid, const unsigned char *data, size_t length, const z_data_info_t *info, void *arg) {    
  Z_UNUSED_ARG_3(length, info, arg);
  z1_sto1_last_res.name = strdup(rid->id.rname);
  z1_sto1_last_res.data = *data;
  z_mvar_put(z1_sto1_mvar, &z1_sto1_last_res);
}

void z2_sto1_listener(const z_resource_id_t *rid, const unsigned char *data, size_t length, const z_data_info_t *info, void *arg) {    
  Z_UNUSED_ARG_3(length, info, arg);
  z2_sto1_last_res.name = strdup(rid->id.rname);
  z2_sto1_last_res.data = *data;
  z_mvar_put(z2_sto1_mvar, &z2_sto1_last_res);
}

void z1_sto1_handler(const char *rname, const char *predicate, z_array_resource_t *replies, void *arg) {
  Z_UNUSED_ARG_3(rname, predicate, arg);
  replies->length = 1;
  replies->elem = (z_resource_t**)malloc(sizeof(z_resource_t *));
  replies->elem[0] = (z_resource_t *)malloc(sizeof(z_resource_t));
  replies->elem[0]->rname = z1_sto1_last_res.name;
  replies->elem[0]->data = &z1_sto1_last_res.data;
  replies->elem[0]->length = 1;
  replies->elem[0]->encoding = 0;
  replies->elem[0]->kind = 0;
}

void z2_sto1_handler(const char *rname, const char *predicate, z_array_resource_t *replies, void *arg) {
  Z_UNUSED_ARG_3(rname, predicate, arg);
  replies->length = 1;
  replies->elem = (z_resource_t**)malloc(sizeof(z_resource_t *));
  replies->elem[0] = (z_resource_t *)malloc(sizeof(z_resource_t));
  replies->elem[0]->rname = z2_sto1_last_res.name;
  replies->elem[0]->data = &z2_sto1_last_res.data;
  replies->elem[0]->length = 1;
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
  resource *res;
  switch (reply->kind) {
    case Z_STORAGE_DATA: 
      res = (resource*)malloc(sizeof(resource));
      res->name = strdup(reply->rname);
      res->data = *reply->data;
      replies = z_list_cons(replies, res);
      break;
    case Z_STORAGE_FINAL:
      break;
    case Z_REPLY_FINAL:
      z_mvar_put(replies_mvar, replies);
      break;
  }
}

int main(int argc, char **argv) {
  Z_UNUSED_ARG_2(argc, argv);
  
  z1_sub1_mvar = z_mvar_empty();
  z2_sub1_mvar = z_mvar_empty();
  z1_sto1_mvar = z_mvar_empty();
  z2_sto1_mvar = z_mvar_empty();
  replies_mvar = z_mvar_empty();

  char *locator = strdup("tcp/127.0.0.1:7447");

  z_zenoh_p_result_t z1_r = z_open(locator, 0, 0);
  ASSERT_RESULT(z1_r, "Unable to open session with broker")
  z_zenoh_t *z1 = z1_r.value.zenoh;
  z_start_recv_loop(z1);

  z_sub_mode_t sm;
  sm.kind = Z_PUSH_MODE;
  z_sub_p_result_t z1_sub1_r = z_declare_subscriber(z1, "/test/client/**", &sm, z1_sub1_listener, NULL);
  ASSERT_P_RESULT(z1_sub1_r,"Unable to declare subscriber\n");

  z_sto_p_result_t z1_sto1_r = z_declare_storage(z1, "/test/client/**", z1_sto1_listener, z1_sto1_handler, replies_cleaner, NULL);
  ASSERT_P_RESULT(z1_sto1_r, "Unable to declare storage\n");

  z_pub_p_result_t z1_pub1_r = z_declare_publisher(z1, "/test/client/z1_pub1");
  ASSERT_P_RESULT(z1_pub1_r, "Unable to declare publisher\n");
  z_pub_t *z1_pub1 = z1_pub1_r.value.pub;

  z_zenoh_p_result_t z2_r = z_open(locator, 0, 0);
  ASSERT_RESULT(z2_r, "Unable to open session with broker")
  z_zenoh_t *z2 = z2_r.value.zenoh;
  z_start_recv_loop(z2);

  z_sub_p_result_t z2_sub1_r = z_declare_subscriber(z2, "/test/client/**", &sm, z2_sub1_listener, NULL);
  ASSERT_P_RESULT(z2_sub1_r,"Unable to declare subscriber\n");

  z_sto_p_result_t z2_sto1_r = z_declare_storage(z1, "/test/client/**", z2_sto1_listener, z2_sto1_handler, replies_cleaner, NULL);
  ASSERT_P_RESULT(z2_sto1_r, "Unable to declare storage\n");

  z_pub_p_result_t z2_pub1_r = z_declare_publisher(z2, "/test/client/z2_pub1");
  ASSERT_P_RESULT(z2_pub1_r, "Unable to declare publisher\n");
  z_pub_t *z2_pub1 = z2_pub1_r.value.pub;

  sleep(1);
  
  resource  sent_res;
  resource *rcvd_res;

  sent_res.name = "/test/client/z1_wr1";
  sent_res.data = 12;
  z_write_data(z1, sent_res.name, &sent_res.data, 1);
  rcvd_res = z_mvar_get(z1_sub1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  rcvd_res = z_mvar_get(z2_sub1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  rcvd_res = z_mvar_get(z1_sto1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  rcvd_res = z_mvar_get(z2_sto1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);

  z_query(z1, "/test/client/**", "", reply_handler, NULL);
  z_mvar_get(replies_mvar);
  assert(2 == z_list_len(replies));
  rcvd_res = z_list_head(replies);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  z_list_free(&replies);

  z_query(z2, "/test/client/**", "", reply_handler, NULL);
  z_mvar_get(replies_mvar);
  assert(2 == z_list_len(replies));
  rcvd_res = z_list_head(replies);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  z_list_free(&replies);


  sent_res.name = "/test/client/z2_wr1";
  sent_res.data = 5;
  z_write_data(z2, sent_res.name, &sent_res.data, 1);
  rcvd_res = z_mvar_get(z1_sub1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  rcvd_res = z_mvar_get(z2_sub1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  rcvd_res = z_mvar_get(z1_sto1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  rcvd_res = z_mvar_get(z2_sto1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);

  z_query(z1, "/test/client/**", "", reply_handler, NULL);
  z_mvar_get(replies_mvar);
  assert(2 == z_list_len(replies));
  rcvd_res = z_list_head(replies);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  z_list_free(&replies);

  z_query(z2, "/test/client/**", "", reply_handler, NULL);
  z_mvar_get(replies_mvar);
  assert(2 == z_list_len(replies));
  rcvd_res = z_list_head(replies);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  z_list_free(&replies);


  sent_res.name = "/test/client/z1_pub1";
  sent_res.data = 23;
  z_stream_data(z1_pub1, &sent_res.data, 1);
  rcvd_res = z_mvar_get(z1_sub1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  rcvd_res = z_mvar_get(z2_sub1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  rcvd_res = z_mvar_get(z1_sto1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  rcvd_res = z_mvar_get(z2_sto1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);

  z_query(z1, "/test/client/**", "", reply_handler, NULL);
  z_mvar_get(replies_mvar);
  assert(2 == z_list_len(replies));
  rcvd_res = z_list_head(replies);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  z_list_free(&replies);

  z_query(z2, "/test/client/**", "", reply_handler, NULL);
  z_mvar_get(replies_mvar);
  assert(2 == z_list_len(replies));
  rcvd_res = z_list_head(replies);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  z_list_free(&replies);


  sent_res.name = "/test/client/z2_pub1";
  sent_res.data = 54;
  z_stream_data(z2_pub1, &sent_res.data, 1);
  rcvd_res = z_mvar_get(z1_sub1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  rcvd_res = z_mvar_get(z2_sub1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  rcvd_res = z_mvar_get(z1_sto1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  rcvd_res = z_mvar_get(z2_sto1_mvar);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);

  z_query(z1, "/test/client/**", "", reply_handler, NULL);
  z_mvar_get(replies_mvar);
  assert(2 == z_list_len(replies));
  rcvd_res = z_list_head(replies);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  z_list_free(&replies);

  z_query(z2, "/test/client/**", "", reply_handler, NULL);
  z_mvar_get(replies_mvar);
  assert(2 == z_list_len(replies));
  rcvd_res = z_list_head(replies);
  assert(0 == strcmp(sent_res.name, rcvd_res->name));
  assert(sent_res.data == rcvd_res->data);
  z_list_free(&replies);
}
