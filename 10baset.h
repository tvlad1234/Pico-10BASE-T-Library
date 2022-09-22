#ifndef __UDP_H__
#define __UDP_H__

#include <stdint.h>

// Buffer max size config
#define MAX_UDP_PAYLOAD_SIZE (64)
#define PACKET_QUEUE_LENGTH (16)

//Destination IP structure
typedef struct dest_ip_t{
    uint8_t  ip1, ip2, ip3, ip4;
} dest_ip_t;


// Start ethernet on core 1
void eth_core_start();

// printf on udp
void udp_printf(dest_ip_t ip, uint16_t port, const char *format, ...);

// set ip address
void eth_set_ip(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4);

#endif //__UDP_H__
