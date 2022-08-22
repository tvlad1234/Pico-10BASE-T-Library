#ifndef __UDP_H__
#define __UDP_H__

#include <stdint.h>

// Start ethernet on core 1
void eth_core_start();

// printf on udp
void udp_printf(const char *format, ...);

// set ip address
void eth_set_ip(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4);

// set destination ip address
void eth_set_dest(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4);

#endif //__UDP_H__
