# Pico-10BASE-T-Library
Library which allows the Pi Pico to send UDP packets over 10BASE-T Ethernet \
Based on [Pico-10BASE-T](https://github.com/kingyoPiyo/Pico-10BASE-T) by kingyoPiyo  \
**Featured on [Hackaday](https://hackaday.com/2022/08/27/an-elegant-ethernet-library-for-your-next-rp2040-project)** \
Example code can be found [here](https://github.com/tvlad1234/Pico-10BASE-Thermometer)

## What does it do?
This library allows the Raspbery Pi Pico (or any other RP2040-based board) to send UDP traffic over Ethernet with no extra hardware. This is acomplished by using the PIO blocks inside the RP2040. This library also makes use of the second core of the microcontroller, which handles the link pulse and data sending. 

## How to use it?
The hardware configuration used here is the same as in [Pico-10BASE-T](https://github.com/kingyoPiyo/Pico-10BASE-T).

In order to use this library, you must add the _Pico-10BASE-T-Library_ subdirectory to the CMakeLists.txt of your project and include _10baset_ in _target_link_libraries_. Then, just add the _10baset.h_ header into your code. 

The library provides the following functions: \
`void eth_core_start()` is used to initialize the Ethernet link on the second core and PIO. \
`void eth_set_ip(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4)` sets the IP address of the board. \
`void eth_set_dest(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4)` sets the IP address of the destination (a receving computer, for example). \
`void udp_set_port(uint16_t port)` sets the UDP port to transmit to. \
`void udp_printf(const char *format, ...)` sends a formatted string to the destination IP address over UDP. It should be used just like a regular printf function, however be mindful of the 64-byte UDP payload size limitation set in [10baset.h](10baset.h).



