# Pico-10BASE-T-Library
Library which allows the Pi Pico to send UDP packets over 10BASE-T Ethernet \
Based on [Pico-10BASE-T](https://github.com/kingyoPiyo/Pico-10BASE-T) by kingyoPiyo  \
Example code can be found [here](https://github.com/tvlad1234/Pico-10BASE-Thermometer)

## What does it do?
This library allows the Raspbery Pi Pico (or any other RP2040-based board) to send UDP traffic over Ethernet with no extra hardware. This is acomplished by using the PIO blocks inside the RP2040. This library also makes use of the second core of the microcontroller, which handles the link pulse and data sending. 

## How to use it?
The hardware configuration used here is the same as in [Pico-10BASE-T](https://github.com/kingyoPiyo/Pico-10BASE-T).

In order to use this library, you must add the _Pico-10BASE-T-Library_ subdirectory to the CMakeLists.txt of your project and include _10baset_ in _target_link_libraries_. Then, just add the _10baset.h_ header into your code. 

The `dest_ip_t` type is a structure which is used to specify the destination IP for the transmit functions. It contains the four bytes of the destination IP: _ip1_, _ip2_, _ip3_, _ip4_.

The library provides the following functions: \
`void eth_core_start()` is used to initialize the Ethernet link on the second core and PIO. \
`void eth_set_ip(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4)` sets the IP address of the board. \
`void udp_printf(dest_ip_t ip, uint16_t port, const char *format, ...)` sends a formatted string to the specified destination IP address on the specified port over UDP. It should be used just like a regular printf function, however be mindful of the UDP payload size limitation set in [10baset.h](10baset.h), which can be modified to suit your own application. \
`void udp_send_data(dest_ip_t ip, uint16_t port, uint8_t data[], uint16_t length)` sends data to the specified IP, over the specified port. It works by packing the data into a payload structure. \
`void udp_send_payload(udp_payload *payload)` transmits a pre-packaged payload.



