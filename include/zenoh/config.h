#ifndef ZENOH_C_CONFIG_H
#define ZENOH_C_CONFIG_H

#define ZENOH_READ_BUF_LEN 65563
#define ZENOH_WRITE_BUF_LEN 65563
#define ZENOH_PID_LENGTH 8
#define ZENOH_PROTO_VERSION 0x01
#define ZENOH_DEFAULT_LEASE 10000


/**
 * ZENOH_DEBUG :
 *  - 2 : ERROR + DEBUG
 *  - 1 : ERROR
 *  - 0 : NOTHING
 */ 
#define ZENOH_DEBUG 2

#define ZENOH_MACOS 1
// #define ZENOH_LINUX 0
// #define ZENOH_CONTIKI 0
// #define ZENOH_MBED 0
// #define ZENOH_WINDOWS 0



#define ZENOH_TRANSPORT_TCP_IP 1
// #define ZENOH_TRANSPORT_BLE 1

#endif /* ZENOH_C_CONFIG_H */