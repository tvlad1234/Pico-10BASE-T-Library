#ifndef __UDP_H__
#define __UDP_H__

#include <stdint.h>

// Buffer max size config
#define MAX_UDP_PAYLOAD_SIZE (384)

// Max packet queue length
#define PACKET_QUEUE_LENGTH (4)

// Destination IP structure
typedef struct dest_ip_t
{
    uint8_t ip1, ip2, ip3, ip4;
} dest_ip_t;


typedef struct udp_payload
{
    dest_ip_t ip;
    uint16_t port;
    uint8_t data[MAX_UDP_PAYLOAD_SIZE];
    uint16_t length;
} udp_payload;

// Set ip address
void eth_set_ip(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4);

// Start ethernet on core 1
void eth_core_start();

// printf over UDP
void udp_printf(dest_ip_t ip, uint16_t port, const char *format, ...);

// Send data over UDP 
void udp_send_data(dest_ip_t ip, uint16_t port, uint8_t data[], uint16_t length);

// Send premade payload over UDP
void udp_send_payload(udp_payload *payload);

#endif //__UDP_H__
