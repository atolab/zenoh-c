#ifndef ZENOH_C_NET_RESULT_H
#define ZENOH_C_NET_RESULT_H

#define ZN_PROPERTY_PARSE_ERROR 0x81
#define ZN_PROPERTIES_PARSE_ERROR 0x82
#define ZN_MESSAGE_PARSE_ERROR 0x83
#define ZN_INSUFFICIENT_IOBUF_SIZE 0x84
#define ZN_IO_ERROR 0x85
#define ZN_RESOURCE_DECL_ERROR 0x86
#define ZN_PAYLOAD_HEADER_PARSE_ERROR 0x87
#define ZN_TX_CONNECTION_ERROR 0x89
#define ZN_INVALID_ADDRESS_ERROR 0x8a
#define ZN_FAILED_TO_OPEN_SESSION 0x8b
#define ZN_UNEXPECTED_MESSAGE 0x8c

#define ZN_RESULT_DECLARE(type, name) RESULT_DECLARE(type, name, zn)

#define _ZN_RESULT_DECLARE(type, name) RESULT_DECLARE(type, name, _zn)

#define ZN_P_RESULT_DECLARE(type, name) P_RESULT_DECLARE(type, name, zn) 

#define _ZN_P_RESULT_DECLARE(type, name) P_RESULT_DECLARE(type, name, _zn) 

#endif /* ZENOH_C_NET_RESULT_H */
