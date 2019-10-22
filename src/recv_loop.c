#include <stdio.h>
#include <stdatomic.h>
#include "zenoh/recv_loop.h"
#include "zenoh/types.h"
#include "zenoh/codec.h"
#include "zenoh/private/net.h"
#include "zenoh/private/msgcodec.h"
#include "zenoh/private/logging.h"


void z_do_nothing() { }

typedef struct {
  z_zenoh_t *z;
  z_vle_t qid;
  z_array_uint8_t qpid;
  atomic_int nb_qhandlers;
  atomic_flag sent_final;
} query_handle_t;

void send_replies(void* query_handle, z_array_resource_t replies, uint8_t eval_flag){
    unsigned int i;
    int rsn = 0;
    query_handle_t *handle = (query_handle_t*)query_handle;
    _z_message_t msg;
    msg.header = _Z_REPLY | _Z_F_FLAG | eval_flag;
    msg.payload.reply.qid = handle->qid;
    msg.payload.reply.qpid = handle->qpid;
    msg.payload.reply.stoid = handle->z->pid;

    for(i = 0; i < replies.length; ++i)
    {
        msg.payload.reply.rsn = rsn++;
        msg.payload.reply.rname = (char *)replies.elem[i]->rname;
        _Z_DEBUG_VA("[%d] - Query reply key: %s\n", i, msg.payload.reply.rname);
        _z_payload_header_t ph;
        ph.flags = _Z_ENCODING | _Z_KIND;
        ph.encoding = replies.elem[i]->encoding;
        ph.kind = replies.elem[i]->kind;
        _Z_DEBUG_VA("[%d] - Payload Length: %zu\n", i, replies.elem[i]->length);
        _Z_DEBUG_VA("[%d] - Payload address: %p\n", i, (void*)replies.elem[i]->data);

        ph.payload = z_iobuf_wrap_wo((unsigned char *)replies.elem[i]->data, replies.elem[i]->length, 0,  replies.elem[i]->length);
        z_iobuf_t buf = z_iobuf_make(replies.elem[i]->length + 32 );
        _z_payload_header_encode(&buf, &ph);
        msg.payload.reply.payload_header = buf;

        if (_z_send_large_msg(handle->z->sock, &handle->z->wbuf, &msg, replies.elem[i]->length + 128) == 0) {
            z_iobuf_free(&buf);
        } else {
            _Z_DEBUG("Trying to reconnect....\n");
            handle->z->on_disconnect(handle->z);
            _z_send_large_msg(handle->z->sock, &handle->z->wbuf, &msg, replies.elem[i]->length + 128);
            z_iobuf_free(&buf);
        }
    }
    msg.payload.reply.rsn = rsn++;
    msg.payload.reply.rname = "";
    z_iobuf_t buf = z_iobuf_make(0);
    msg.payload.reply.payload_header = buf;

    if (_z_send_msg(handle->z->sock, &handle->z->wbuf, &msg) == 0) {
        z_iobuf_free(&buf);
    } else {
        _Z_DEBUG("Trying to reconnect....\n");
        handle->z->on_disconnect(handle->z);
        _z_send_msg(handle->z->sock, &handle->z->wbuf, &msg);
        z_iobuf_free(&buf);
    }

    atomic_fetch_sub(&handle->nb_qhandlers, 1);
    if(handle->nb_qhandlers <= 0 && !atomic_flag_test_and_set(&handle->sent_final))
    {
        msg.header = _Z_REPLY;

        if (_z_send_msg(handle->z->sock, &handle->z->wbuf, &msg) != 0) {
            _Z_DEBUG("Trying to reconnect....\n");
            handle->z->on_disconnect(handle->z);
            _z_send_msg(handle->z->sock, &handle->z->wbuf, &msg);
        }    
    }
}

void send_eval_replies(void* query_handle, z_array_resource_t replies){
    send_replies(query_handle, replies, _Z_E_FLAG);
}

void send_storage_replies(void* query_handle, z_array_resource_t replies){
    send_replies(query_handle, replies, 0);
}

void handle_msg(z_zenoh_t *z, _z_message_p_result_t r) {
    _z_payload_header_result_t r_ph;
    z_data_info_t info;
    _z_declaration_t * decls;
    z_resource_id_t rid;    
    const char *rname;
    uint8_t mid;    
    z_list_t *subs;
    z_list_t *stos;
    z_list_t *evals;
    z_list_t *lit;
    _z_sub_t *sub;
    _z_sto_t *sto;
    _z_eva_t *eval;
    _z_replywaiter_t *rw;    
    z_reply_value_t rvalue; 
    _z_res_decl_t *rd;
    unsigned int i;
    rname = 0;
    subs = z_list_empty;
    stos = z_list_empty;
    lit = z_list_empty;
    mid = _Z_MID(r.value.message->header);
    switch (mid) {
        case _Z_STREAM_DATA:
            _Z_DEBUG_VA("Received _Z_STREAM_DATA message %d\n", Z_MID(r.value.message->header));
            rname = _z_get_resource_name(z, r.value.message->payload.stream_data.rid);
            if (rname != 0) {
                subs = _z_get_subscriptions_by_rname(z, rname);
                stos = _z_get_storages_by_rname(z, rname);
                rid.kind = Z_STR_RES_ID;
                rid.id.rname = (char *)rname;
            } else {
                subs = _z_get_subscriptions_by_rid(z, r.value.message->payload.stream_data.rid);
                rid.kind = Z_INT_RES_ID;
                rid.id.rid = r.value.message->payload.stream_data.rid;
            }
            
            if (subs != 0 || stos != 0) {
                _z_payload_header_decode_na(&r.value.message->payload.stream_data.payload_header, &r_ph);
                if (r_ph.tag == Z_OK_TAG) {
                    info.flags = r_ph.value.payload_header.flags;
                    info.encoding = r_ph.value.payload_header.encoding;
                    info.kind = r_ph.value.payload_header.kind;
                    if (info.flags & _Z_T_STAMP) {
                        info.tstamp = r_ph.value.payload_header.tstamp;
                    }
                    lit = subs;
                    while (lit != z_list_empty) {
                        sub = z_list_head(lit);
                        sub->data_handler(&rid, r_ph.value.payload_header.payload.buf, z_iobuf_readable(&r_ph.value.payload_header.payload), &info, sub->arg);
                        lit = z_list_tail(lit);
                    }
                    lit = stos;
                    while (lit != z_list_empty) {
                        sto = z_list_head(lit);
                        sto->data_handler(&rid, r_ph.value.payload_header.payload.buf, z_iobuf_readable(&r_ph.value.payload_header.payload), &info, sto->arg);
                        lit = z_list_tail(lit);
                    }
                    free(r_ph.value.payload_header.payload.buf);
                    z_list_free(&subs);
                    z_list_free(&stos);
                }
                else {
                    do {
                        _Z_DEBUG("Unable to parse StreamData Message Payload Header\n");
                    } while (0);
                }
                
            } else {
                _Z_DEBUG_VA("No subscription found for resource %zu\n", r.value.message->payload.stream_data.rid);
            }
            z_iobuf_free(&r.value.message->payload.stream_data.payload_header);
            break;
        case _Z_COMPACT_DATA:
            _Z_DEBUG_VA("Received _Z_COMPACT_DATA message %d\n", Z_MID(r.value.message->header));
            rname = _z_get_resource_name(z, r.value.message->payload.stream_data.rid);
            if (rname != 0) {
                subs = _z_get_subscriptions_by_rname(z, rname);
                stos = _z_get_storages_by_rname(z, rname);
                rid.kind = Z_STR_RES_ID;
                rid.id.rname = (char *)rname;
            } else {
                subs = _z_get_subscriptions_by_rid(z, r.value.message->payload.stream_data.rid);
                rid.kind = Z_INT_RES_ID;
                rid.id.rid = r.value.message->payload.stream_data.rid;
            }
            
            if (subs != 0 || stos != 0) {
                info.flags = 0;
                lit = subs;
                while (lit != z_list_empty) {
                    sub = z_list_head(lit);
                    sub->data_handler(
                        &rid,
                        r.value.message->payload.compact_data.payload.buf,
                        z_iobuf_readable(&r.value.message->payload.compact_data.payload),
                        &info,
                        sub->arg);
                    lit = z_list_tail(lit);
                }
                lit = stos;
                while (lit != z_list_empty) {
                    sto = z_list_head(lit);
                    sto->data_handler(
                        &rid,
                        r.value.message->payload.compact_data.payload.buf,
                        z_iobuf_readable(&r.value.message->payload.compact_data.payload),
                        &info,
                        sto->arg);
                    lit = z_list_tail(lit);
                }
                free(r.value.message->payload.compact_data.payload.buf);
                z_list_free(&subs);
            }
            break;
        case _Z_WRITE_DATA:
            _Z_DEBUG_VA("Received _Z_WRITE_DATA message %d\n", Z_MID(r.value.message->header));
            subs = _z_get_subscriptions_by_rname(z, r.value.message->payload.write_data.rname);
            stos = _z_get_storages_by_rname(z, r.value.message->payload.write_data.rname);
            if (subs != 0 || stos != 0) {
                rid.kind = Z_STR_RES_ID;
                rid.id.rname = r.value.message->payload.write_data.rname;
                _Z_DEBUG("Decoding Payload Header");
                _z_payload_header_decode_na(&r.value.message->payload.write_data.payload_header, &r_ph);
                if (r_ph.tag == Z_OK_TAG) {
                    info.flags = r_ph.value.payload_header.flags;
                    info.encoding = r_ph.value.payload_header.encoding;
                    info.kind = r_ph.value.payload_header.kind;
                    if (info.flags & _Z_T_STAMP) {
                        info.tstamp = r_ph.value.payload_header.tstamp;
                    }
                    while (subs != z_list_empty) {
                        sub = (_z_sub_t *) z_list_head(subs);
                        sub->data_handler(
                            &rid,
                            r_ph.value.payload_header.payload.buf,
                            z_iobuf_readable(&r_ph.value.payload_header.payload),
                            &info,
                            sub->arg);
                        subs = z_list_tail(subs);
                    }
                    while (stos != z_list_empty) {
                        sto = (_z_sto_t *) z_list_head(stos);
                        sto->data_handler(
                            &rid,
                            r_ph.value.payload_header.payload.buf,
                            z_iobuf_readable(&r_ph.value.payload_header.payload),
                            &info,
                            sto->arg);
                        stos = z_list_tail(stos);
                    }
                    free(r_ph.value.payload_header.payload.buf);
                }
                else
                    _Z_DEBUG("Unable to parse WriteData Message Payload Header\n");
            } else {
                _Z_DEBUG_VA("No subscription found for resource %s\n", r.value.message->payload.write_data.rname);
            }
            z_iobuf_free(&r.value.message->payload.write_data.payload_header);
            break;
        case _Z_QUERY:
            _Z_DEBUG("Received _Z_QUERY message\n");
            stos = _z_get_storages_by_rname(z, r.value.message->payload.query.rname);
            evals = _z_get_evals_by_rname(z, r.value.message->payload.query.rname);
            if (stos != 0 || evals != 0) {
                query_handle_t *query_handle = malloc(sizeof(query_handle_t));
                query_handle->z = z;
                query_handle->qid = r.value.message->payload.query.qid;
                query_handle->qpid = r.value.message->payload.query.pid;

                int nb_qhandlers = 0;
                if(stos != 0){nb_qhandlers += z_list_len(stos);}
                if(evals != 0){nb_qhandlers += z_list_len(evals);}
                atomic_init(&query_handle->nb_qhandlers, nb_qhandlers);
                atomic_flag_clear(&query_handle->sent_final);

                if(stos != 0){
                    lit = stos;
                    while (lit != z_list_empty) {
                        sto = (_z_sto_t *) z_list_head(lit);
                        sto->query_handler(
                            r.value.message->payload.query.rname,
                            r.value.message->payload.query.predicate,
                            send_storage_replies, 
                            query_handle,
                            sto->arg);
                        lit = z_list_tail(lit);
                    }
                    z_list_free(&stos);
                }

                if(evals != 0){
                    lit = evals;
                    while (lit != z_list_empty) {
                        eval = (_z_eva_t *) z_list_head(lit);
                        eval->query_handler(
                            r.value.message->payload.query.rname,
                            r.value.message->payload.query.predicate,
                            send_eval_replies, 
                            query_handle,
                            eval->arg);
                        lit = z_list_tail(lit);
                    }
                    z_list_free(&evals);
                }
            }
            break;
        case _Z_REPLY:
            _Z_DEBUG("Received _Z_REPLY message\n");
            rw = _z_get_query(z, r.value.message->payload.reply.qid);
            if (rw != 0) {
                if (r.value.message->header & _Z_F_FLAG) {
                    rvalue.srcid = r.value.message->payload.reply.stoid.elem;
                    rvalue.srcid_length = r.value.message->payload.reply.stoid.length;
                    rvalue.rsn = r.value.message->payload.reply.rsn;
                    if (strlen(r.value.message->payload.reply.rname) != 0) {
                        rvalue.rname = r.value.message->payload.reply.rname;
                        _z_payload_header_decode_na(&r.value.message->payload.reply.payload_header, &r_ph);
                        if (r_ph.tag == Z_OK_TAG) {
                            rvalue.info.flags = r_ph.value.payload_header.flags;
                            rvalue.info.encoding = r_ph.value.payload_header.encoding;
                            rvalue.info.kind = r_ph.value.payload_header.kind;
                            if (rvalue.info.flags & _Z_T_STAMP) {
                                rvalue.info.tstamp = r_ph.value.payload_header.tstamp;
                            }
                            rvalue.data = r_ph.value.payload_header.payload.buf;
                            rvalue.data_length = z_iobuf_readable(&r_ph.value.payload_header.payload);
                        }
                        else {
                            _Z_DEBUG("Unable to parse Reply Message Payload Header\n");
                            break;
                        }
                        if (r.value.message->header & _Z_E_FLAG) {
                            rvalue.kind = Z_EVAL_DATA;
                        }else{
                            rvalue.kind = Z_STORAGE_DATA;
                        }
                    } else {
                        if (r.value.message->header & _Z_E_FLAG) {
                            rvalue.kind = Z_EVAL_FINAL;
                        }else{
                            rvalue.kind = Z_STORAGE_FINAL;
                        }
                    }
                } else {
                    rvalue.kind = Z_REPLY_FINAL;
                }
                rw->reply_handler(&rvalue, rw->arg);
                
                switch (rvalue.kind) {
                    case Z_STORAGE_DATA:
                        free((void *)rvalue.data);
                        free((void *)rvalue.rname);
                        free((void *)rvalue.srcid);
                        break;
                    case Z_STORAGE_FINAL:
                        free((void *)rvalue.srcid);
                        break;
                    case Z_REPLY_FINAL:
                        break;
                    default:
                        break;
                }
            }
            break;
        case _Z_DECLARE:
            _Z_DEBUG("Received _Z_DECLARE message\n");
            decls = r.value.message->payload.declare.declarations.elem;
            for (i = 0; i < r.value.message->payload.declare.declarations.length; ++i) {
                switch (_Z_MID(decls[i].header)) {
                    case _Z_RESOURCE_DECL:
                        _Z_DEBUG("Received declare-resource message\n");
                        _z_register_res_decl(z, decls[i].payload.resource.rid,  decls[i].payload.resource.r_name);
                        break;
                    case _Z_PUBLISHER_DECL:
                        break;
                    case _Z_SUBSCRIBER_DECL:
                        _Z_DEBUG_VA("Registering remote subscription for resource: %zu\n", decls[i].payload.sub.rid);
                        rd = _z_get_res_decl_by_rid(z, decls[i].payload.sub.rid);
                        if (rd != 0)
                            z_i_map_set(z->remote_subs, decls[i].payload.sub.rid, rd);
                        break;
                    case _Z_COMMIT_DECL:
                        break;

                    case _Z_RESULT_DECL:
                        break;
                    case _Z_FORGET_RESOURCE_DECL:
                    case _Z_FORGET_PUBLISHER_DECL:
                    case _Z_FORGET_SUBSCRIBER_DECL:
                    case _Z_FORGET_SELECTION_DECL:
                    case _Z_STORAGE_DECL:
                    case _Z_FORGET_STORAGE_DECL:
                    case _Z_EVAL_DECL:
                    case _Z_FORGET_EVAL_DECL:
                    default:
                        break;
                }
            }
            break;
        case _Z_ACCEPT:
            _Z_DEBUG("Received _Z_ACCEPT message\n");
            break;
        case _Z_CLOSE:
            _Z_DEBUG("Received _Z_CLOSE message\n");
            break;
        default:
            break;
    }
}

void* z_recv_loop(z_zenoh_t *z) {
    _z_message_p_result_t r;
    z_vle_result_t r_vle;
    z_iobuf_t bigbuf;
    z_iobuf_t *buf;
    _z_message_p_result_init(&r);
    int jump_to;
    z_iobuf_clear(&z->rbuf);
    bigbuf.capacity = 0;
    z->running = 1;
    while (z->running) {

    // READ SIZE
    if (z_iobuf_readable(&z->rbuf) < 4) {
        z_iobuf_compact(&z->rbuf);
        if (_z_recv_buf(z->sock, &z->rbuf) <= 0) return 0;
    }
    r_vle = z_vle_decode(&z->rbuf);

    // ALLOCATE BIG BUFFER IF NEEDED
    if(r_vle.value.vle > ZENOH_READ_BUF_LEN - 10) {
        bigbuf = z_iobuf_make(r_vle.value.vle);
        int length = z_iobuf_readable(&z->rbuf);
        z_iobuf_write_bytes(&bigbuf, z_iobuf_read_n(&z->rbuf, length), length);
        buf = &bigbuf;
    }
    else {
        buf = &z->rbuf;
    }

    // READ MESSAGE
    if (r_vle.value.vle > z_iobuf_readable(buf)) {
        z_iobuf_compact(&z->rbuf);
        do {
            if (_z_recv_buf(z->sock, buf) <= 0) return 0;
        } while (r_vle.value.vle > z_iobuf_readable(buf));
    }
    jump_to = buf->r_pos + r_vle.value.vle;

    // DECODE MESSAGE
    _z_message_decode_na(buf, &r);
    if (r.tag == Z_OK_TAG) {
        handle_msg(z, r);
    } else {
        _Z_DEBUG("Connection closed due to receive error");
        return 0;
    }

    if(bigbuf.capacity > 0) {
        z_iobuf_free(&bigbuf);
    }
    else {
        // Ensure we jump to the next message if if we did not parse the message.
        z->rbuf.r_pos = jump_to;
    }
    }
    return 0;
}

int z_running(z_zenoh_t *z) { 
    return z->running;
}
