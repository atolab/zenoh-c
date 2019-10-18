*******
zenoh-c
*******
The *libzenoh-c* library provides a C client API for the zenoh protocol.

API reference
=============

Functions
---------

.. c:function:: z_zenoh_p_result_t z_open(char* locator, z_on_disconnect_t on_disconnect, const z_vec_t *ps)

  Open a zenoh session with the infrastructure component (zenoh router, zenoh broker, ...) reachable 
  at location *locator*. *locator* is a string representation of a network endpoint. A typical 
  locator looks like this : ``tcp/127.0.0.1:7447``. *on_disconnect* is a function that will be called 
  each time the client API is disconnected from the infrastructure. It can be set to ``NULL``. 
  *ps* is a set of *z_property_t* that will be used to establish and configure the zenoh session. 
  *ps* will typically contain the ``username`` and ``password`` informations needed to establish the zenoh 
  session with a secured infrastructure. It can be set to ``NULL``. Return a handle to the zenoh session.

.. c:function:: z_vec_t z_info(z_zenoh_t *z)

  Return a set of *z_property_t* containing various informations about the established zenoh session 
  represented by *z*.

.. c:function:: z_sub_p_result_t z_declare_subscriber(z_zenoh_t *z, const char* resource, const z_sub_mode_t *sm, z_data_handler_t data_handler, void *arg)

  Declare a subscribtion for all published data matching the provided resource *resource* in session *z*. 
  *sm* is the subscription mode. *data_handler* is the callback function that will be called each time a 
  data matching the subscribed *resource* is received. *arg* is an argument hat will be passed to the 
  *data_handler* on each call. Return a zenoh subscriber.

.. c:function:: z_pub_p_result_t z_declare_publisher(z_zenoh_t *z, const char *resource)

  Declare a publication for resource *resource* in session *z*.
  Return a zenoh publisher.
  
.. c:function:: z_sto_p_result_t z_declare_storage(z_zenoh_t *z, const char* resource, z_data_handler_t data_handler, z_query_handler_t query_handler, void *arg)

  Declare a storage for all data matching the provided resource *resource* in session *z*. 
  *data_handler* is the callback function that will be called each time a data matching the stored 
  *resource* is received. *query_handler* is the callback function that will be called each time a 
  query for data matching the stored *resource* is received. The *query_handler* function must 
  call the provided *send_replies* function with the resulting data. *send_replies* can be called 
  with an empty array. *arg* is an argument hat will be passed to the *data_handler* and the 
  *query_handler* on each call. Return a zenoh storage.

.. c:function:: z_eval_p_result_t z_declare_eval(z_zenoh_t *z, const char* resource, z_query_handler_t query_handler, void *arg)
  
  Declare an eval able to provide data matching the provided resource *resource* in session *z*. 
  *query_handler* is the callback function that will be called each time a query for data matching 
  the evaluated *resource* is received. The *query_handler* function must call the provided *send_replies* 
  function with the resulting data. *send_replies* can be called with an empty array. 
  *arg* is an argument hat will be passed to the *query_handler* function on each call. 
  Return a zenoh eval.

.. c:function:: int z_stream_compact_data(z_pub_t *pub, const unsigned char *payload, size_t len)

  Send data in a compact_data message for the resource published by publisher *pub*. *payload* is a pointer 
  to the data to be sent. *len* is the size of the data to be sent. Return 0 if the publication was successful.

.. c:function:: int z_stream_data(z_pub_t *pub, const unsigned char *payload, size_t len)

  Send data in a stream_data message for the resource published by publisher *pub*. *payload* is a pointer 
  to the data to be sent. *len* is the size of the data to be sent. Return 0 if the publication was successful.

.. c:function:: int z_stream_data_wo(z_pub_t *pub, const unsigned char *payload, size_t len, uint8_t encoding, uint8_t kind)

  Send data in a stream_data message for the resource published by publisher *pub*. *payload* is a pointer 
  to the data to be sent. *len* is the size of the data to be sent. *encoding* is a metadata information 
  associated with the published data that represents the encoding of the published data. *kind* is a 
  metadata information associated with the published data that represents the kind of publication.
  Return 0 if the publication was successful.

.. c:function:: int z_write_data(z_zenoh_t *z, const char* resource, const unsigned char *payload, size_t length)

  Send data in a write_data message for the resource *resource*. *payload* is a pointer to the data to be sent. 
  *len* is the size of the data to be sent. Return 0 if the publication was successful.

.. c:function:: int z_write_data_wo(z_zenoh_t *z, const char* resource, const unsigned char *payload, size_t length, uint8_t encoding, uint8_t kind)

  Send data in a write_data message for the resource *resource*. *payload* is a pointer to the data to be sent. 
  *len* is the size of the data to be sent. *encoding* is a metadata information associated with the published 
  data that represents the encoding of the published data. *kind* is a metadata information associated with 
  the published data that represents the kind of publication. Return 0 if the publication was successful.

.. c:function:: int z_pull(z_sub_t *sub)

  Pull data for the `Z_PULL_MODE` or `Z_PERIODIC_PULL_MODE` subscribtion *sub*. The pulled data will be provided 
  by calling the *data_handler* function provided to the `c.z_declare_subscriber`_ function.

.. c:function:: int z_query(z_zenoh_t *z, const char* resource, const char* predicate, z_reply_handler_t reply_handler, void *arg)

  Query data matching resource *resource* in session *z*. *predicate* is a string that will be 
  propagated to the storages and evals that should provide the queried data. It may allow them to 
  filter, transform and/or compute the queried data. *reply_handler* is the callback function that 
  will be called on reception of the replies of the query. *arg* is an argument hat will be passed 
  to the *reply_handler* function on each call. Return 0 if the query was sent successfully.

.. c:function:: int z_query_wo(z_zenoh_t *z, const char* resource, const char* predicate, z_reply_handler_t reply_handler, void *arg, z_query_dest_t dest_storages, z_query_dest_t dest_evals)

  Query data matching resoure *resource* in session *z*. *predicate* is a string that will be 
  propagated to the storages and evals that should provide the queried data. It may allow them to 
  filter, transform and/or compute the queried data. *reply_handler* is the callback function that 
  will be called on reception of the replies of the query. *arg* is an argument hat will be passed 
  to the *reply_handler* function on each call. *dest_storages* indicates which matching storages 
  should be destination of the query. *dest_evals* indicates which matching evals should be 
  destination of the query. Return 0 if the query was sent successfully.

.. c:function:: int z_undeclare_subscriber(z_sub_t *sub)

  Undeclare the subscrbtion *sub*.
  Return 0 when successful.

.. c:function:: int z_undeclare_publisher(z_sub_t *pub)

  Undeclare the publication *pub*.
  Return 0 when successful.

.. c:function:: int z_undeclare_storage(z_sub_t *sto)

  Undeclare the storage *sto*.
  Return 0 when successful.

.. c:function:: int z_undeclare_eval(z_sub_t *eval)

  Undeclare the eval *eval*.
  Return 0 when successful.

.. c:function:: int z_close(z_zenoh_t *z)

  Close the zenoh session *z*.
  Return 0 when successful.

