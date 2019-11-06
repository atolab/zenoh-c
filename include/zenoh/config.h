#ifndef ZENOH_C_CONFIG_H
#define ZENOH_C_CONFIG_H

#define ZENOH_READ_BUF_LEN 65563
#define ZENOH_WRITE_BUF_LEN 65563
#define ZENOH_PID_LENGTH 8
#define ZENOH_PROTO_VERSION 0x01
#define ZENOH_DEFAULT_LEASE 10000

#define ZENOH_SCOUT_ADDR "239.255.0.1"
#define ZENOH_LOCAL_HOST "127.0.0.1"
#define ZENOH_MAX_SCOUT_MSG_LEN 1024
#define ZENOH_SCOUT_PORT 7447

// #define ZENOH_MACOS 1 

#define ZENOH_TRANSPORT_TCP_IP 1
// #define ZENOH_TRANSPORT_BLE 1

#endif /* ZENOH_C_CONFIG_H */
