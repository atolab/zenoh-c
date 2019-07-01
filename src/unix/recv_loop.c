#include <stdio.h>
#include "zenoh/types.h"
#include "zenoh/net.h"
#include "zenoh/codec.h"
#include "zenoh/unix/recv_loop.h"
#include "zenoh/recv_loop.h"



void* z_recv_loop(void* arg) {
    z_zenoh_t *z = (z_zenoh_t*)arg;
    z_runtime_t *rt = (z_runtime_t*)z->runtime;
    z_message_p_result_t r;
    z_vle_result_t r_vle;
    z_payload_header_result_t r_ph;
    z_data_info_t info;
    z_declaration_t * decls;
    z_message_p_result_init(&r);
    z_resource_id_t rid;    
    uint8_t mid;    
    z_list_t *subs;
    z_list_t *stos;
    z_list_t *lit;
    z_subscription_t *sub;
    z_storage_t *sto;
    z_replywaiter_t *rw;    
    z_reply_value_t rvalue; 
    z_array_z_resource_t replies;
    z_res_decl_t *rd;
    int i;
    int rsn;
    int jump_to;
    const char *rname;
    int rb;
    z_iobuf_clear(&z->rbuf);
    while (rt->running) {
        rname = 0;       
        subs = z_list_empty;
        stos = z_list_empty;
        lit = z_list_empty;        
        if (z_iobuf_readable(&z->rbuf) < 4) {
            // printf("Not enough data on buffer, reading Socket\n");
            // printf("Buffer r_pos = %d, w_pos = %d before compact\n", z->rbuf.r_pos, z->rbuf.w_pos);
            z_iobuf_compact(&z->rbuf);
            // printf("Buffer r_pos = %d, w_pos = %d AFTER  compact\n", z->rbuf.r_pos, z->rbuf.w_pos);
            rb = z_recv_buf(z->sock, &z->rbuf);
            // printf("Buffer r_pos = %d, w_pos = %d AFTER  read\n", z->rbuf.r_pos, z->rbuf.w_pos);
            // printf("Reveived %d bytes\n", rb);
        }        
        r_vle = z_vle_decode(&z->rbuf);        
        // printf("Decoding message of %zu bytes\n", r_vle.value.vle);        
        if (r_vle.value.vle > z_iobuf_readable(&z->rbuf)) {
            // printf("Incomplete messasge on buffer, reading Socket\n");
            z_iobuf_compact(&z->rbuf);
            do {
                z_recv_buf(z->sock, &z->rbuf);                
            } while (r_vle.value.vle > z_iobuf_readable(&z->rbuf));            
        }
        jump_to = z->rbuf.r_pos + r_vle.value.vle;

        z_message_decode_na(&z->rbuf, &r);        
        // z_recv_msg_na(z->sock, &z->rbuf, &r);
        // printf("Tag = %d\n", r.tag);
        if (r.tag == Z_OK_TAG) {
            mid = Z_MID(r.value.message->header);
            switch (mid) {    
                case Z_STREAM_DATA:          
                    Z_DEBUG_VA("Received Z_STREAM_DATA message %d\n", Z_MID(r.value.message->header));          
                    rname = z_get_resource_name(z, r.value.message->payload.stream_data.rid);
                    if (rname != 0) {
                        subs = z_get_subscriptions_by_rname(z, rname);
                        stos = z_get_storages_by_rname(z, rname);
                        rid.kind = Z_STR_RES_ID;
                        rid.id.rname = (char *)rname;
                    } else {
                        subs = z_get_subscriptions_by_rid(z, r.value.message->payload.stream_data.rid);
                        rid.kind = Z_INT_RES_ID;
                        rid.id.rid = r.value.message->payload.stream_data.rid;
                    }
                    
                    if (subs != 0 || stos != 0) {
                        z_payload_header_decode_na(&r.value.message->payload.stream_data.payload_header, &r_ph);
                        if (r_ph.tag == Z_OK_TAG) {
                            info.flags = r_ph.value.payload_header.flags;                            
                            info.encoding = r_ph.value.payload_header.encoding;
                            info.kind = r_ph.value.payload_header.kind; 
                            lit = subs;                    
                            while (lit != z_list_empty) {
                                sub = z_list_head(lit);
                                sub->callback(&rid, r_ph.value.payload_header.payload.buf, z_iobuf_readable(&r_ph.value.payload_header.payload), info, sub->arg);
                                lit = z_list_tail(lit);
                            }
                            lit = stos;
                            while (lit != z_list_empty) {
                                sto = z_list_head(lit);
                                sto->callback(&rid, r_ph.value.payload_header.payload.buf, z_iobuf_readable(&r_ph.value.payload_header.payload), info, sto->arg);
                                lit = z_list_tail(lit);
                            }
                            free(r_ph.value.payload_header.payload.buf);
                            z_list_free(&subs);
                            z_list_free(&stos);
                        }                
                        else                                                         
                            Z_DEBUG("Unable to parse StreamData Message Payload Header\n");          
                        
                    } else {
                        Z_DEBUG_VA("No subscription found for resource %zu\n", r.value.message->payload.stream_data.rid);          
                    }                     
                    z_iobuf_free(&r.value.message->payload.stream_data.payload_header);                        
                    break;
                case Z_COMPACT_DATA:              
                    Z_DEBUG_VA("Received Z_COMPACT_DATA message %d\n", Z_MID(r.value.message->header));                
                    rname = z_get_resource_name(z, r.value.message->payload.stream_data.rid);
                    if (rname != 0) {
                        subs = z_get_subscriptions_by_rname(z, rname);
                        stos = z_get_storages_by_rname(z, rname);
                        rid.kind = Z_STR_RES_ID;
                        rid.id.rname = (char *)rname;
                    } else {
                        subs = z_get_subscriptions_by_rid(z, r.value.message->payload.stream_data.rid);
                        rid.kind = Z_INT_RES_ID;
                        rid.id.rid = r.value.message->payload.stream_data.rid;
                    }
                    
                    if (subs != 0 || stos != 0) {
                        info.flags = 0;
                        lit = subs; 
                        while (lit != z_list_empty) {
                            sub = z_list_head(lit);
                            sub->callback(
                                &rid,
                                r.value.message->payload.compact_data.payload.buf, 
                                z_iobuf_readable(&r.value.message->payload.compact_data.payload),
                                info,
                                sub->arg);                     
                            lit = z_list_tail(lit);
                        }
                        lit = stos;
                        while (lit != z_list_empty) {
                            sto = z_list_head(lit);
                            sto->callback(
                                &rid,
                                r.value.message->payload.compact_data.payload.buf, 
                                z_iobuf_readable(&r.value.message->payload.compact_data.payload),
                                info,
                                sto->arg);
                            lit = z_list_tail(lit);
                        }                        
                        free(r.value.message->payload.compact_data.payload.buf);       
                        z_list_free(&subs);
                    }                     
                    break;
                case Z_WRITE_DATA:                             
                    Z_DEBUG_VA("Received Z_WRITE_DATA message %d\n", Z_MID(r.value.message->header)); 
                    subs = z_get_subscriptions_by_rname(z, r.value.message->payload.write_data.rname);
                    stos = z_get_storages_by_rname(z, r.value.message->payload.write_data.rname);
                    if (subs != 0 || stos != 0) {
                        rid.kind = Z_STR_RES_ID;
                        rid.id.rname = r.value.message->payload.write_data.rname;
                        Z_DEBUG("Decoding Payload Header");
                        z_payload_header_decode_na(&r.value.message->payload.write_data.payload_header, &r_ph);                    
                        if (r_ph.tag == Z_OK_TAG) {
                            info.flags = r_ph.value.payload_header.flags;                            
                            info.encoding = r_ph.value.payload_header.encoding;
                            info.kind = r_ph.value.payload_header.kind;     
                            while (subs != z_list_empty) {
                                sub = (z_subscription_t *) z_list_head(subs);
                                sub->callback(
                                    &rid, 
                                    r_ph.value.payload_header.payload.buf, 
                                    z_iobuf_readable(&r_ph.value.payload_header.payload), 
                                    info,
                                    sub->arg);
                                subs = z_list_tail(subs);
                            }
                            while (stos != z_list_empty) {
                                sto = (z_storage_t *) z_list_head(stos);                                                                                                           
                                sto->callback(
                                    &rid, 
                                    r_ph.value.payload_header.payload.buf, 
                                    z_iobuf_readable(&r_ph.value.payload_header.payload), 
                                    info,
                                    sto->arg);
                                stos = z_list_tail(stos);
                            }
                            free(r_ph.value.payload_header.payload.buf);
                        }                                                
                        else                         
                            Z_DEBUG("Unable to parse WriteData Message Payload Header\n");          
                    } else {
                        Z_DEBUG_VA("No subscription found for resource %s\n", r.value.message->payload.write_data.rname);          
                    }                     
                    z_iobuf_free(&r.value.message->payload.write_data.payload_header);                        
                    break;                    
                case Z_QUERY:
                    Z_DEBUG("Received Z_QUERY message\n");
                    stos = z_get_storages_by_rname(z, r.value.message->payload.query.rname);
                    if (stos != 0) {
                        z_message_t msg;
                        msg.header = Z_REPLY | Z_F_FLAG;
                        msg.payload.reply.qid = r.value.message->payload.query.qid;
                        msg.payload.reply.qpid = r.value.message->payload.query.pid;
                        while (stos != z_list_empty) {
                            rsn = 0;
                            sto = (z_storage_t *) z_list_head(stos);
                            replies = sto->handler(
                                r.value.message->payload.query.rname,
                                r.value.message->payload.query.predicate,
                                sto-> arg);
                            msg.payload.reply.stoid = z->pid;
                            for(i = 0; i < replies.length; ++i)
                            {
                                msg.payload.reply.rsn = rsn++;
                                msg.payload.reply.rname = (char *)replies.elem[i].rname;
                                z_payload_header_t ph;
                                ph.flags = Z_ENCODING | Z_KIND;
                                ph.encoding = replies.elem[i].encoding;
                                ph.kind = replies.elem[i].kind;
                                ph.payload = z_iobuf_wrap_wo((unsigned char *)replies.elem[i].data, replies.elem[i].length, 0,  replies.elem[i].length);
                                z_iobuf_t buf = z_iobuf_make(replies.elem[i].length + 32 );
                                z_payload_header_encode(&buf, &ph);
                                msg.payload.reply.payload_header = buf;
  
                                if (z_send_msg(z->sock, &z->wbuf, &msg) == 0) {
                                    z_iobuf_free(&buf);
                                } else { 
                                    Z_DEBUG("Trying to reconnect....\n");
                                    z->on_disconnect(z);
                                    z_send_msg(z->sock, &z->wbuf, &msg);
                                    z_iobuf_free(&buf);
                                }
                            }
                            msg.payload.reply.rsn = rsn++;
                            msg.payload.reply.rname = "";
                            z_iobuf_t buf = z_iobuf_make(0);
                            msg.payload.reply.payload_header = buf;

                            if (z_send_msg(z->sock, &z->wbuf, &msg) == 0) {
                                z_iobuf_free(&buf);
                            } else { 
                                Z_DEBUG("Trying to reconnect....\n");
                                z->on_disconnect(z);
                                z_send_msg(z->sock, &z->wbuf, &msg);
                                z_iobuf_free(&buf);
                            }
                            sto->cleaner(replies, sto->arg);

                            stos = z_list_tail(stos);
                        }
                        msg.header = Z_REPLY;
                        msg.payload.reply.qid = r.value.message->payload.query.qid;
                        msg.payload.reply.qpid = r.value.message->payload.query.pid;

                        if (z_send_msg(z->sock, &z->wbuf, &msg) != 0) {
                            Z_DEBUG("Trying to reconnect....\n");
                            z->on_disconnect(z);
                            z_send_msg(z->sock, &z->wbuf, &msg);
                        }
                    }
                    break;
                case Z_REPLY:
                    Z_DEBUG("Received Z_REPLY message\n");
                    rw = z_get_query(z, r.value.message->payload.reply.qid);
                    if (rw != 0) {
                        if (r.value.message->header & Z_F_FLAG) {
                            rvalue.stoid = r.value.message->payload.reply.stoid.elem;
                            rvalue.stoid_length = r.value.message->payload.reply.stoid.length;
                            rvalue.rsn = r.value.message->payload.reply.rsn;
                            if (strlen(r.value.message->payload.reply.rname) != 0) {
                                rvalue.rname = r.value.message->payload.reply.rname;
                                z_payload_header_decode_na(&r.value.message->payload.reply.payload_header, &r_ph);
                                if (r_ph.tag == Z_OK_TAG) {
                                    rvalue.info.flags = r_ph.value.payload_header.flags;
                                    rvalue.info.encoding = r_ph.value.payload_header.encoding;
                                    rvalue.info.kind = r_ph.value.payload_header.kind;
                                    rvalue.data = r_ph.value.payload_header.payload.buf;
                                    rvalue.data_length = z_iobuf_readable(&r_ph.value.payload_header.payload);
                                }
                                else {
                                    Z_DEBUG("Unable to parse Reply Message Payload Header\n");
                                    break;
                                }
                                rvalue.kind = Z_STORAGE_DATA;
                            } else {
                                rvalue.kind = Z_STORAGE_FINAL;
                            }
                        } else {
                            rvalue.kind = Z_REPLY_FINAL;
                        }
                        rw->callback(&rvalue, rw->arg);
                    }
                    break;
                case Z_DECLARE:       
                    Z_DEBUG("Received Z_DECLARE message\n");
                    decls = r.value.message->payload.declare.declarations.elem; 
                    for (int i = 0; i < r.value.message->payload.declare.declarations.length; ++i) {
                        switch (Z_MID(decls[i].header)) {
                            case Z_RESOURCE_DECL: 
                                Z_DEBUG("Received declare-resource message\n");
                                z_register_res_decl(z, decls[i].payload.resource.rid,  decls[i].payload.resource.r_name);
                                break;
                            case Z_PUBLISHER_DECL:
                                break;
                            case Z_SUBSCRIBER_DECL:             
                                Z_DEBUG_VA("Registering remote subscription for resource: %zu\n", decls[i].payload.sub.rid);
                                rd = z_get_res_decl_by_rid(z, decls[i].payload.sub.rid);
                                if (rd != 0) 
                                    z_i_map_set(z->remote_subs, decls[i].payload.sub.rid, rd);                                
                                break;
                            case Z_COMMIT_DECL:
                                break;

                            case Z_RESULT_DECL:
                                break;
                            case Z_FORGET_RESOURCE_DECL:
                            case Z_FORGET_PUBLISHER_DECL:
                            case Z_FORGET_SUBSCRIBER_DECL:
                            case Z_FORGET_SELECTION_DECL:
                            case Z_STORAGE_DECL:
                            case Z_FORGET_STORAGE_DECL:
                            default:
                                break;
                        }
                    }
                    break;
                case Z_ACCEPT:
                    break;
                default:
                    break;
            }                        
        } else {
            Z_DEBUG("Connection closed due to receive error");
            return 0;
        }
        // Ensure we jump to the next message if if we did not parse the message.
        z->rbuf.r_pos = jump_to;
    }
    return 0;
}


int z_start_recv_loop(z_zenoh_t* z) { 
    z_runtime_t *zr = (z_runtime_t*)malloc(sizeof(z_runtime_t));    
    bzero(zr, sizeof(z_runtime_t));
    z->runtime = zr;    
    if (pthread_create(&(zr->recv_loop), 0, z_recv_loop, z) == 0) {
        zr->running = 1;
        return 0;
    } 
    return -1;
}


int z_stop_recv_loop(z_zenoh_t *z) { 
    z_message_t c;
    c.header = Z_CLOSE;
    c.payload.close.pid = z->pid;
    c.payload.close.reason = Z_PEER_CLOSE;
    z_send_msg(z->sock, &z->wbuf, &c);
    close(z->sock);
    ((z_runtime_t*)z->runtime)->running = 0;
    return 0;
}