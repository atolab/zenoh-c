
#include "zenoh/types.h"
#include "zenoh/net.h"
#include "zenoh/unix/recv_loop.h"
#include "zenoh/recv_loop.h"



void* z_recv_loop(void* arg) {
    zenoh_t *z = (zenoh_t*)arg;
    z_runtime_t *rt = (z_runtime_t*)z->runtime;
    z_message_p_result_t r;
    z_message_p_result_init(&r);
    z_resource_id_t rid;    
    uint8_t mid;
    z_subscription_t *sub;
    while (rt->running) {
        z_recv_msg_na(z->sock, &z->rbuf, &r);
        if (r.tag == Z_OK_TAG) {
            mid = Z_MID(r.value.message->header);
            switch (mid) {    
                case Z_STREAM_DATA:          
                    Z_DEBUG_VA("Received message %d\n", Z_MID(r.value.message->header));          
                    sub = z_get_subscription(z, r.value.message->payload.stream_data.rid);
                    if (sub != 0) {
                        rid.kind = Z_INT_RES_ID;
                        rid.id.rid = r.value.message->payload.stream_data.rid;
                        sub->callback(mid, rid,
                                    r.value.message->payload.stream_data.payload_header);                                              
                    } else {
                        Z_DEBUG_VA("No subscription found for resource %llu\n", r.value.message->payload.stream_data.rid);          
                    }                     
                    z_iobuf_free(&r.value.message->payload.stream_data.payload_header);                        
                    break;
                case Z_COMPACT_DATA:                    
                    sub = z_get_subscription(z, r.value.message->payload.stream_data.rid);
                    if (sub != 0) {
                        rid.kind = Z_INT_RES_ID;
                        rid.id.rid = r.value.message->payload.stream_data.rid;
                        sub->callback(mid, rid,
                                    r.value.message->payload.compact_data.payload);                        
                    } 
                    z_iobuf_free(&r.value.message->payload.compact_data.payload);
                    break;
                case Z_WRITE_DATA:          
                    Z_DEBUG_VA("Unsupported %d\n", Z_MID(r.value.message->header));          
                    // Z_DEBUG_VA("Received message %d\n", Z_MID(r.value.message->header));          
                    // sub = z_get_subscription(z, r.value.message->payload.stream_data.rid);
                    // if (sub != 0) {
                    //     rid.kind = Z_INT_RES_ID;
                    //     rid.id.rid = r.value.message->payload.stream_data.rid;
                    //     sub->callback(mid, rid,
                    //                 r.value.message->payload.stream_data.payload_header);                                              
                    // } else {
                    //     Z_DEBUG_VA("No subscription found for resource %llu\n", r.value.message->payload.stream_data.rid);          
                    // }                     
                    // z_iobuf_free(&r.value.message->payload.stream_data.payload_header);                        
                    break;                    
                case Z_DECLARE:
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
    }
    return 0;
}


int z_start_recv_loop(zenoh_t* z) { 
    z_runtime_t *zr = (z_runtime_t*)malloc(sizeof(z_runtime_t));    
    bzero(zr, sizeof(z_runtime_t));
    z->runtime = zr;    
    if (pthread_create(&(zr->recv_loop), 0, z_recv_loop, z) == 0) {
        zr->running = 1;
        return 0;
    } 
    return -1;
}


int z_stop_recv_loop(zenoh_t *z) { 
    z_message_t c;
    c.header = Z_CLOSE;
    c.payload.close.pid = z->pid;
    c.payload.close.reason = Z_PEER_CLOSE;
    z_send_msg(z->sock, &z->wbuf, &c);
    close(z->sock);
    ((z_runtime_t*)z->runtime)->running = 0;
    return 0;
}