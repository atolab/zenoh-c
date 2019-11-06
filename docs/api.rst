*************
API Reference
*************

Result types
------------

Result types are datastructures that allow API functions to return a value when they succeed 
or an error code when they fail. 
  
.. c:type:: struct z_name_result_t
  
  A data structure containing either a value of type *z_name_t* either an error code.

  .. c:member:: enum result_kind tag

    One of the following result kinds:

      | ``Z_OK_TAG``
      | ``Z_ERROR_TAG``

  .. c:member:: union value

    .. c:member:: type <name> 

      The actual value when :c:member:`z_name_result_t.tag` equals ``Z_OK_TAG``.

    .. c:member:: int error

      The error code when :c:member:`z_name_result_t.tag` equals ``Z_ERROR_TAG``.
    
.. c:type:: struct z_name_p_result_t
  
  A data structure containing either a pointer to a value of type *z_name_t* either an error code.

  .. c:member:: enum result_kind tag

    One of the following result kinds:

      | ``Z_OK_TAG``
      | ``Z_ERROR_TAG``

  .. c:member:: union value

    .. c:member:: type *<name> 

      A pointer to the actual value when :c:member:`z_name_p_result_t.tag` equals ``Z_OK_TAG``.

    .. c:member:: int error

      The error code when :c:member:`z_name_p_result_t.tag` equals ``Z_ERROR_TAG``.


Collections
-----------

Arrays
~~~~~~

.. c:type:: struct z_array_name_t

  An array of elemets of type *z_name_t*. 

  .. c:member:: unsigned int length

    The length of the array.

  .. c:member:: z_name_t *elem

    The *z_name_t* values.


.. c:type:: struct z_array_p_name_t

  An array of pointers to elemets of type *z_name_t*. 

  .. c:member:: unsigned int length

    The length of the array.

  .. c:member:: z_name_t *elem

    The pointers to the *z_name_t* values.


Vectors
~~~~~~~

.. c:type:: struct z_vec_t

  A sequence container that encapsulates a dynamic size array of pointers. 

  .. c:member:: unsigned int capacity_

    The maximum capacity of the vector.

  .. c:member:: unsigned int length_

    The current length of the vector.

  .. c:member:: void **elem_

    The pointers to the values.

.. c:function:: z_vec_t z_vec_make(unsigned int capacity)

  Initialize a :c:type:`z_vec_t` with a :c:member:`z_vec_t.capacity_` of **capacity**, 
  a :c:member:`z_vec_t.length_` of **0** and a :c:member:`z_vec_t.elem_` pointing to a 
  newly allocated array of **capacity** pointers.

.. c:function:: unsigned int z_vec_length(const z_vec_t* v)

  Return the current length of the given :c:type:z_vec_t.

.. c:function:: void z_vec_append(z_vec_t* v, void* e) 

  Append the element **e** to the vector **v** and take ownership of the appended element.

.. c:function:: void z_vec_set(z_vec_t* sv, unsigned int i, void* e)

  Set the element **e** in the vector **v** at index **i** and take ownership of the element.

.. c:function:: const void* z_vec_get(const z_vec_t* v, unsigned int i)

  Return the element at index **i** in vector **v**.


Data Structures
---------------

.. c:type:: struct z_resource_id_t

  Data structure representing a resource id.

  .. c:member:: int kind

    One of the following kinds:

      | ``Z_INT_RES_ID``
      | ``Z_STR_RES_ID``

  .. c:member:: union z_res_id_t id

    .. c:member:: z_vle_t rid

      A numerical resource id when :c:member:`z_resource_id_t.kind` equals ``Z_INT_RES_ID``.

    .. c:member:: char *rname

      A string resource id when :c:member:`z_resource_id_t.kind` equals ``Z_STR_RES_ID``.

.. c:type:: struct z_sub_mode_t

  Data structure representing a subscription mode (see :c:func:`z_declare_subscriber`).

  .. c:member:: uint8_t kind

    One of the following subscription modes:

      | ``Z_PUSH_MODE``
      | ``Z_PULL_MODE``
      | ``Z_PERIODIC_PUSH_MODE``
      | ``Z_PERIODIC_PULL_MODE``

  .. c:member:: z_temporal_property_t tprop

    The period. *Unsupported*

.. c:type:: struct z_timestamp_t

  Data structure representing a unique timestamp.

  .. c:member:: z_vle_t time

    The time.

  .. c:member:: uint8_t clock_id[16]

    The unique identifyer of the clock that generated this timestamp.

.. c:type:: struct z_data_info_t

  Data structure containing meta informations about the associated data.

  .. c:member:: unsigned int flags

    Flags indicating which meta information is present in the :c:type:`z_data_info_t`: 
    
      | ``_Z_T_STAMP``
      | ``_Z_KIND``
      | ``_Z_ENCODING``

  .. c:member:: z_timestamp_t tstamp
    
    The unique timestamp at which the data has been produced.

  .. c:member:: uint8_t encoding

    The encoding of the data.

  .. c:member:: unsigned short kind

    The kind of the data.

.. c:type:: struct z_query_dest_t

  Data structure defining which storages or evals should be destination of a query (see :c:func:`z_query_wo`).

  .. c:member:: uint8_t kind

    One of the following destination kinds: 

      | ``Z_BEST_MATCH`` the nearest complete storage/eval if there is one, all storages/evals if not.
      | ``Z_COMPLETE`` only complete storages/evals. 
      | ``Z_ALL`` all storages/evals.
      | ``Z_NONE`` no storages/evals.

  .. c:member:: uint8_t nb

    The number of storages or evals that should be destination of the query when 
    :c:member:`z_query_dest_t.kind` equals ``Z_COMPLETE``.

.. c:type:: struct z_reply_value_t

  Data structure containing one of the replies to a query (see :c:type:`z_reply_handler_t`).

  .. c:member:: char kind

    One of the following kinds:

      | ``Z_STORAGE_DATA`` the reply contains some data from a storage.
      | ``Z_STORAGE_FINAL`` the reply indicates that no more data is expected from the specified storage.
      | ``Z_EVAL_DATA`` the reply contains some data from an eval.
      | ``Z_EVAL_FINAL`` the reply indicates that no more data is expected from the specified eval.
      | ``Z_REPLY_FINAL`` the reply indicates that no more replies are expected for the query.

  .. c:member:: const unsigned char *srcid

    The unique identifier of the storage or eval that sent the reply when :c:member:`z_reply_value_t.kind` equals 
    ``Z_STORAGE_DATA``, ``Z_STORAGE_FINAL``, ``Z_EVAL_DATA`` or ``Z_EVAL_FINAL``.

  .. c:member:: size_t srcid_length

    The length of the :c:member:`z_reply_value_t.srcid` when :c:member:`z_reply_value_t.kind` equals 
    ``Z_STORAGE_DATA``, ``Z_STORAGE_FINAL``, ``Z_EVAL_DATA`` or ``Z_EVAL_FINAL``.

  .. c:member:: z_vle_t rsn

    The sequence number of the reply from the identified storage or eval when :c:member:`z_reply_value_t.kind` equals 
    ``Z_STORAGE_DATA``, ``Z_STORAGE_FINAL``, ``Z_EVAL_DATA`` or ``Z_EVAL_FINAL``. 
  
  .. c:member:: const char *rname

    The resource name of the received data when :c:member:`z_reply_value_t.kind` equals 
    ``Z_STORAGE_DATA`` or ``Z_EVAL_DATA``.

  .. c:member:: const unsigned char *data

    A pointer to the received data when :c:member:`z_reply_value_t.kind` equals 
    ``Z_STORAGE_DATA`` or ``Z_EVAL_DATA``.

  .. c:member:: size_t data_length

    The length of the received :c:member:`z_reply_value_t.data` when :c:member:`z_reply_value_t.kind` equals 
    ``Z_STORAGE_DATA`` or ``Z_EVAL_DATA``.

  .. c:member:: z_data_info_t info

    Some meta information about the received :c:member:`z_reply_value_t.data` when :c:member:`z_reply_value_t.kind` equals 
    ``Z_STORAGE_DATA`` or ``Z_EVAL_DATA``.

.. c:type:: struct z_property_t

  A key/value pair where the key is an integer and the value a byte sequence.

  .. c:member:: z_vle_t id

    The key of the :c:type:`z_property_t`.

  .. c:member:: z_array_uint8_t value

    The value of the :c:type:`z_property_t`.

Functions
---------

.. c:function:: z_zenoh_p_result_t z_open(char* locator, z_on_disconnect_t on_disconnect, const z_vec_t *ps)

  Open a zenoh session with the infrastructure component (zenoh router, zenoh broker, ...) reachable at location **locator**. 
  
  | **locator** is a string representation of a network endpoint. A typical locator looks like this : ``tcp/127.0.0.1:7447``. 
  | **on_disconnect** is a function that will be called each time the client API is disconnected from the infrastructure. It can be set to ``NULL``. 
  | **ps** is a :c:type:`vector<z_vec_t>` of :c:type:`z_property_t` that will be used to establish and configure the zenoh session. 
    **ps** will typically contain the ``username`` and ``password`` informations needed to establish the zenoh session with a secured infrastructure. 
    It can be set to ``NULL``. 
  
  Return a handle to the zenoh session.

.. c:function:: z_vec_t z_info(z_zenoh_t *z)

  Return a :c:type:`vector<z_vec_t>` of :c:type:`z_property_t` containing various informations about the established zenoh session 
  represented by **z**.

.. c:function:: z_sub_p_result_t z_declare_subscriber(z_zenoh_t *z, const char* resource, const z_sub_mode_t *sm, z_data_handler_t data_handler, void *arg)

  Declare a subscribtion for all published data matching the provided resource **resource** in session **z**. 
  
  | **z** is the zenoh session.
  | **resource** is the resource to subscribe to.
  | **sm** is the subscription mode. 
  | **data_handler** is the callback function that will be called each time a data matching the subscribed **resource** is received. 
  | **arg** is a pointer that will be passed to the **data_handler** on each call. 
  
  Return a zenoh subscriber.

.. c:function:: z_pub_p_result_t z_declare_publisher(z_zenoh_t *z, const char *resource)

  Declare a publication for resource **resource** in session **z**.

  | **z** is the zenoh session.
  | **resource** is the resource name to publish.
  
  Return a zenoh publisher.
  
.. c:function:: z_sto_p_result_t z_declare_storage(z_zenoh_t *z, const char* resource, z_data_handler_t data_handler, z_query_handler_t query_handler, void *arg)

  Declare a storage for all data matching the provided resource **resource** in session **z**. 
  
  | **z** is the zenoh session.
  | **resource** is the resource to store.
  | **data_handler** is the callback function that will be called each time a data matching the stored **resource** is received. 
  | **query_handler** is the callback function that will be called each time a query for data matching the stored **resource** is received. 
    The **query_handler** function MUST call the provided **send_replies** function with the resulting data. **send_replies** can be called with an empty array. 
  | **arg** is a pointer that will be passed to the **data_handler** and the **query_handler** on each call. 
  
  Return a zenoh storage.

.. c:function:: z_eval_p_result_t z_declare_eval(z_zenoh_t *z, const char* resource, z_query_handler_t query_handler, void *arg)
  
  Declare an eval able to provide data matching the provided resource **resource** in session **z**. 
  
  | **z** is the zenoh session.
  | **resource** is the resource to evaluate.
  | **query_handler** is the callback function that will be called each time a query for data matching the evaluated **resource** is received.
    The **query_handler** function MUST call the provided **send_replies** function with the resulting data. **send_replies** can be called with an empty array. 
  | **arg** is a pointer that will be passed to the **query_handler** function on each call. 
  
  Return a zenoh eval.

.. c:function:: int z_stream_compact_data(z_pub_t *pub, const unsigned char *payload, size_t len)

  Send data in a *compact_data* message for the resource published by publisher **pub**. 
  
  | **pub** is the publisher to use to send data. 
  | **payload** is a pointer to the data to be sent. 
  | **len** is the size of the data to be sent. 
  
  Return 0 if the publication was successful.

.. c:function:: int z_stream_data(z_pub_t *pub, const unsigned char *payload, size_t len)

  Send data in a *stream_data* message for the resource published by publisher **pub**. 
  
  | **pub** is the publisher to use to send data. 
  | **payload** is a pointer to the data to be sent. 
  | **len** is the size of the data to be sent. 
  
  Return 0 if the publication was successful.

.. c:function:: int z_stream_data_wo(z_pub_t *pub, const unsigned char *payload, size_t len, uint8_t encoding, uint8_t kind)

  Send data in a *stream_data* message for the resource published by publisher **pub**. 
  
  | **pub** is the publisher to use to send data. 
  | **payload** is a pointer to the data to be sent. 
  | **len** is the size of the data to be sent. 
  | **encoding** is a metadata information associated with the published data that represents the encoding of the published data. 
  | **kind** is a metadata information associated with the published data that represents the kind of publication.
  
  Return 0 if the publication was successful.

.. c:function:: int z_write_data(z_zenoh_t *z, const char* resource, const unsigned char *payload, size_t length)

  Send data in a *write_data* message for the resource **resource**. 
  
  | **z** is the zenoh session.
  | **resource** is the resource name of the data to be sent.
  | **payload** is a pointer to the data to be sent. 
  | **len** is the size of the data to be sent. 
  
  Return 0 if the publication was successful.

.. c:function:: int z_write_data_wo(z_zenoh_t *z, const char* resource, const unsigned char *payload, size_t length, uint8_t encoding, uint8_t kind)

  Send data in a *write_data* message for the resource **resource**. 
  
  | **z** is the zenoh session.
  | **resource** is the resource name of the data to be sent.
  | **payload** is a pointer to the data to be sent. 
  | **len** is the size of the data to be sent. 
  | **encoding** is a metadata information associated with the published data that represents the encoding of the published data. 
  | **kind** is a metadata information associated with the published data that represents the kind of publication.
  
  Return 0 if the publication was successful.

.. c:function:: int z_pull(z_sub_t *sub)

  Pull data for the `Z_PULL_MODE` or `Z_PERIODIC_PULL_MODE` subscribtion **sub**. The pulled data will be provided 
  by calling the **data_handler** function provided to the `c.z_declare_subscriber`_ function.

  | **sub** is the subscribtion to pull from.
  
  Return 0 if pull was successful.

.. c:function:: int z_query(z_zenoh_t *z, const char* resource, const char* predicate, z_reply_handler_t reply_handler, void *arg)

  Query data matching resource **resource** in session **z**. 
  
  | **z** is the zenoh session.
  | **resource** is the resource to query.
  | **predicate** is a string that will be  propagated to the storages and evals that should provide the queried data. 
    It may allow them to filter, transform and/or compute the queried data. 
  | **reply_handler** is the callback function that will be called on reception of the replies of the query. 
  | **arg** is a pointer that will be passed to the **reply_handler** function on each call. 
  
  Return 0 if the query was sent successfully.

.. c:function:: int z_query_wo(z_zenoh_t *z, const char* resource, const char* predicate, z_reply_handler_t reply_handler, void *arg, z_query_dest_t dest_storages, z_query_dest_t dest_evals)

  Query data matching resource **resource** in session **z**. 
  
  | **z** is the zenoh session.
  | **resource** is the resource to query.
  | **predicate** is a string that will be  propagated to the storages and evals that should provide the queried data. 
    It may allow them to filter, transform and/or compute the queried data. 
  | **reply_handler** is the callback function that will be called on reception of the replies of the query. 
  | **arg** is a pointer that will be passed to the **reply_handler** function on each call. 
  | **dest_storages** indicates which matching storages should be destination of the query (see :c:type:`z_query_dest_t`).
  | **dest_evals** indicates which matching evals should be destination of the query (see :c:type:`z_query_dest_t`).
  
  Return 0 if the query was sent successfully.

.. c:function:: int z_undeclare_subscriber(z_sub_t *sub)

  Undeclare the subscrbtion **sub**.
  
  | **sub** is the subscription to undeclare.

  Return 0 when successful.

.. c:function:: int z_undeclare_publisher(z_sub_t *pub)

  Undeclare the publication *pub*.
  
  | **pub** is the publication to undeclare.

  Return 0 when successful.

.. c:function:: int z_undeclare_storage(z_sub_t *sto)

  Undeclare the storage **sto**.
  
  | **sto** is the storage to undeclare.

  Return 0 when successful.

.. c:function:: int z_undeclare_eval(z_sub_t *eval)

  Undeclare the eval **eval**.
  
  | **eval** is the eval to undeclare.

  Return 0 when successful.

.. c:function:: int z_close(z_zenoh_t *z)

  Close the zenoh session *z*.
  
  | **z** is the zenoh session to close.

  Return 0 when successful.


Handlers
--------

.. c:type:: void (*z_data_handler_t)(const z_resource_id_t *rid, const unsigned char *data, size_t length, const z_data_info_t *info, void *arg)

  Function to pass as argument of :c:func:`z_declare_subscriber` or :c:func:`z_declare_storage`. 
  It will be called on reception of data matching the subscribed/stored resource. 

  | **rid** is the resource id of the received data.
  | **data** is a pointer to the received data.
  | **length** is the length of the received data.
  | **info** is the :c:type:`z_data_info_t` associated with the received data.
  | **arg** is the pointer passed to :c:func:`z_declare_subscriber` or :c:func:`z_declare_storage`.

.. c:type:: void (*z_query_handler_t)(const char *rname, const char *predicate, z_replies_sender_t send_replies, void *query_handle, void *arg)

  Function to pass as argument of :c:func:`z_declare_storage` or :c:func:`z_declare_eval`.
  It will be called on reception of query matching the stored/evaluated resource. 
  The :c:type:`z_query_handler_t` must provide the data matching the resource *rname* by calling 
  the *send_replies* function with the *query_handle* and the data as arguments. The *send_replies* 
  function MUST be called but accepts empty data array. 
  
  | **rname** is the resource name of the queried data.
  | **predicate** is a string provided by the querier refining the data to be provided.
  | **send_replies** is a function that MUST be called with the *query_handle* and the provided data as arguments.
  | **query_handle** is a pointer to pass as argument of *send_replies*.
  | **arg** is the pointer passed to :c:func:`z_declare_storage` or :c:func:`z_declare_eval`.

.. c:type:: void (*z_reply_handler_t)(const z_reply_value_t *reply, void *arg)

  Function to pass as argument of :c:func:`z_query` or :c:func:`z_query_wo`. 
  It will be called on reception of replies to the query sent by :c:func:`z_query` or :c:func:`z_query_wo`. 
  
  | **reply** is the actual :c:type:`reply<z_reply_value_t>`.
  | **arg** is the pointer passed to :c:func:`z_query` or :c:func:`z_query_wo`. 

.. c:type:: void (*z_on_disconnect_t)(void *z)

  Function to pass as argument of :c:func:`z_open`. 
  It will be called each time the client API is disconnected from the infrastructure.
