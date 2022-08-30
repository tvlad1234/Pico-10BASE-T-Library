/********************************************************
* Title    : Pico-10BASE-T Sample
* Date     : 2022/08/22
* Note     : GP16 TX -
             GP17 TX +
* Design   : kingyo

* Modified by : tvlad1234
to be used as a library

********************************************************/

#include "10baset.h"

#include "pico/stdlib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "pico/unique_id.h"

#include "hardware/pio.h"
#include "hardware/irq.h"

#include "ser_10base_t.pio.h"

// Ethernet
#define DEF_ETH_DST_MAC (0XFFFFFFFFFFFF) // Destination MAC Address
#define DEF_ETH_SRC_MAC (0xE45F01000000) // RasPico MAC Address, starting with Raspberry Pi Foundation OUI.
// a MAC address unique to each board gets generated when initalizing the Ethernet stuff, by filling the zeroes with the last three bytes of the flash serial number.

uint64_t mac_src = DEF_ETH_SRC_MAC;

// IP Header
#define DEF_IP_ADR_SRC1 (192) // RasPico IP Address
#define DEF_IP_ADR_SRC2 (168)
#define DEF_IP_ADR_SRC3 (131)
#define DEF_IP_ADR_SRC4 (176)

#define DEF_IP_DST_DST1 (192) // Destination IP Address
#define DEF_IP_DST_DST2 (168)
#define DEF_IP_DST_DST3 (131)
#define DEF_IP_DST_DST4 (112)

// UDP Header
#define DEF_UDP_SRC_PORTNUM (1234)
#define DEF_UDP_DST_PORTNUM (1234)

uint8_t ip_src1 = DEF_IP_ADR_SRC1;
uint8_t ip_src2 = DEF_IP_ADR_SRC2;
uint8_t ip_src3 = DEF_IP_ADR_SRC3;
uint8_t ip_src4 = DEF_IP_ADR_SRC4;

uint8_t ip_dst1 = DEF_IP_DST_DST1;
uint8_t ip_dst2 = DEF_IP_DST_DST2;
uint8_t ip_dst3 = DEF_IP_DST_DST3;
uint8_t ip_dst4 = DEF_IP_DST_DST4;

uint16_t src_port = DEF_UDP_SRC_PORTNUM;
uint16_t dest_port = DEF_UDP_DST_PORTNUM;

PIO pio_ser_wr = pio0;

queue_t udp_payload_queue;

void udp_init(void);
void udp_packet_gen_10base(uint32_t *buf, uint8_t *udp_payload, uint16_t payloadLength);

// Manchester table
// input 8bit, output 32bit, LSB first
// b00 -> IDLE
// b01 -> LOW
// b10 -> HIGH
// b11 -> not use.
const static uint32_t tbl_manchester[256] = {
    0x66666666,
    0x66666669,
    0x66666696,
    0x66666699,
    0x66666966,
    0x66666969,
    0x66666996,
    0x66666999,
    0x66669666,
    0x66669669,
    0x66669696,
    0x66669699,
    0x66669966,
    0x66669969,
    0x66669996,
    0x66669999,
    0x66696666,
    0x66696669,
    0x66696696,
    0x66696699,
    0x66696966,
    0x66696969,
    0x66696996,
    0x66696999,
    0x66699666,
    0x66699669,
    0x66699696,
    0x66699699,
    0x66699966,
    0x66699969,
    0x66699996,
    0x66699999,
    0x66966666,
    0x66966669,
    0x66966696,
    0x66966699,
    0x66966966,
    0x66966969,
    0x66966996,
    0x66966999,
    0x66969666,
    0x66969669,
    0x66969696,
    0x66969699,
    0x66969966,
    0x66969969,
    0x66969996,
    0x66969999,
    0x66996666,
    0x66996669,
    0x66996696,
    0x66996699,
    0x66996966,
    0x66996969,
    0x66996996,
    0x66996999,
    0x66999666,
    0x66999669,
    0x66999696,
    0x66999699,
    0x66999966,
    0x66999969,
    0x66999996,
    0x66999999,
    0x69666666,
    0x69666669,
    0x69666696,
    0x69666699,
    0x69666966,
    0x69666969,
    0x69666996,
    0x69666999,
    0x69669666,
    0x69669669,
    0x69669696,
    0x69669699,
    0x69669966,
    0x69669969,
    0x69669996,
    0x69669999,
    0x69696666,
    0x69696669,
    0x69696696,
    0x69696699,
    0x69696966,
    0x69696969,
    0x69696996,
    0x69696999,
    0x69699666,
    0x69699669,
    0x69699696,
    0x69699699,
    0x69699966,
    0x69699969,
    0x69699996,
    0x69699999,
    0x69966666,
    0x69966669,
    0x69966696,
    0x69966699,
    0x69966966,
    0x69966969,
    0x69966996,
    0x69966999,
    0x69969666,
    0x69969669,
    0x69969696,
    0x69969699,
    0x69969966,
    0x69969969,
    0x69969996,
    0x69969999,
    0x69996666,
    0x69996669,
    0x69996696,
    0x69996699,
    0x69996966,
    0x69996969,
    0x69996996,
    0x69996999,
    0x69999666,
    0x69999669,
    0x69999696,
    0x69999699,
    0x69999966,
    0x69999969,
    0x69999996,
    0x69999999,
    0x96666666,
    0x96666669,
    0x96666696,
    0x96666699,
    0x96666966,
    0x96666969,
    0x96666996,
    0x96666999,
    0x96669666,
    0x96669669,
    0x96669696,
    0x96669699,
    0x96669966,
    0x96669969,
    0x96669996,
    0x96669999,
    0x96696666,
    0x96696669,
    0x96696696,
    0x96696699,
    0x96696966,
    0x96696969,
    0x96696996,
    0x96696999,
    0x96699666,
    0x96699669,
    0x96699696,
    0x96699699,
    0x96699966,
    0x96699969,
    0x96699996,
    0x96699999,
    0x96966666,
    0x96966669,
    0x96966696,
    0x96966699,
    0x96966966,
    0x96966969,
    0x96966996,
    0x96966999,
    0x96969666,
    0x96969669,
    0x96969696,
    0x96969699,
    0x96969966,
    0x96969969,
    0x96969996,
    0x96969999,
    0x96996666,
    0x96996669,
    0x96996696,
    0x96996699,
    0x96996966,
    0x96996969,
    0x96996996,
    0x96996999,
    0x96999666,
    0x96999669,
    0x96999696,
    0x96999699,
    0x96999966,
    0x96999969,
    0x96999996,
    0x96999999,
    0x99666666,
    0x99666669,
    0x99666696,
    0x99666699,
    0x99666966,
    0x99666969,
    0x99666996,
    0x99666999,
    0x99669666,
    0x99669669,
    0x99669696,
    0x99669699,
    0x99669966,
    0x99669969,
    0x99669996,
    0x99669999,
    0x99696666,
    0x99696669,
    0x99696696,
    0x99696699,
    0x99696966,
    0x99696969,
    0x99696996,
    0x99696999,
    0x99699666,
    0x99699669,
    0x99699696,
    0x99699699,
    0x99699966,
    0x99699969,
    0x99699996,
    0x99699999,
    0x99966666,
    0x99966669,
    0x99966696,
    0x99966699,
    0x99966966,
    0x99966969,
    0x99966996,
    0x99966999,
    0x99969666,
    0x99969669,
    0x99969696,
    0x99969699,
    0x99969966,
    0x99969969,
    0x99969996,
    0x99969999,
    0x99996666,
    0x99996669,
    0x99996696,
    0x99996699,
    0x99996966,
    0x99996969,
    0x99996996,
    0x99996999,
    0x99999666,
    0x99999669,
    0x99999696,
    0x99999699,
    0x99999966,
    0x99999969,
    0x99999996,
    0x99999999,
};

static uint32_t crc_table[256];
static uint16_t ip_identifier = 0;
static uint32_t ip_chk_sum1, ip_chk_sum2, ip_chk_sum3;

// Etherent Frame
static const uint16_t eth_type = 0x0800; // IP

// IPv4 Header
static const uint8_t ip_version = 4; // IP v4
static const uint8_t ip_head_len = 5;
static const uint8_t ip_type_of_service = 0;

static void _make_crc_table(void)
{
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t c = i;
        for (uint32_t j = 0; j < 8; j++)
        {
            c = c & 1 ? (c >> 1) ^ 0xEDB88320 : (c >> 1);
        }
        crc_table[i] = c;
    }
}

void udp_init(void)
{
    _make_crc_table();
}

void udp_set_port(uint16_t port)
{
    dest_port = port;
    src_port = port;
}

void udp_packet_gen_10base(uint32_t *buf, uint8_t *udp_payload, uint16_t payloadLength)
{
    uint16_t udp_chksum = 0;
    uint32_t i, j, idx = 0, ans;

    uint16_t udp_buf_size = payloadLength + 54;
    uint16_t udpLen = payloadLength + 8;
    uint16_t ip_total_len = 20 + udpLen;

    uint8_t *data_8b = calloc(udp_buf_size, sizeof(uint8_t));

    // Calculate the ip check sum
    ip_chk_sum1 = 0x0000C512 + ip_identifier + ip_total_len + (ip_src1 << 8) + ip_src2 + (ip_src3 << 8) + ip_src4 +
                  (ip_dst1 << 8) + ip_dst2 + (ip_dst3 << 8) + ip_dst4;
    ip_chk_sum2 = (ip_chk_sum1 & 0x0000FFFF) + (ip_chk_sum1 >> 16);
    ip_chk_sum3 = ~((ip_chk_sum2 & 0x0000FFFF) + (ip_chk_sum2 >> 16));

    //==========================================================================
    ip_identifier++;

    // Preamble
    for (i = 0; i < 7; i++)
    {
        data_8b[idx++] = 0x55;
    }
    // SFD
    data_8b[idx++] = 0xD5;
    // Destination MAC Address
    data_8b[idx++] = (DEF_ETH_DST_MAC >> 40) & 0xFF;
    data_8b[idx++] = (DEF_ETH_DST_MAC >> 32) & 0xFF;
    data_8b[idx++] = (DEF_ETH_DST_MAC >> 24) & 0xFF;
    data_8b[idx++] = (DEF_ETH_DST_MAC >> 16) & 0xFF;
    data_8b[idx++] = (DEF_ETH_DST_MAC >> 8) & 0xFF;
    data_8b[idx++] = (DEF_ETH_DST_MAC >> 0) & 0xFF;
    // Source MAC Address
    data_8b[idx++] = (mac_src >> 40) & 0xFF;
    data_8b[idx++] = (mac_src >> 32) & 0xFF;
    data_8b[idx++] = (mac_src >> 24) & 0xFF;
    data_8b[idx++] = (mac_src >> 16) & 0xFF;
    data_8b[idx++] = (mac_src >> 8) & 0xFF;
    data_8b[idx++] = (mac_src >> 0) & 0xFF;
    // Ethernet Type
    data_8b[idx++] = (eth_type >> 8) & 0xFF;
    data_8b[idx++] = (eth_type >> 0) & 0xFF;
    // IP Header
    data_8b[idx++] = (ip_version << 4) | (ip_head_len & 0x0F);
    data_8b[idx++] = (ip_type_of_service >> 0) & 0xFF;
    data_8b[idx++] = (ip_total_len >> 8) & 0xFF;
    data_8b[idx++] = (ip_total_len >> 0) & 0xFF;
    data_8b[idx++] = (ip_identifier >> 8) & 0xFF;
    data_8b[idx++] = (ip_identifier >> 0) & 0xFF;
    data_8b[idx++] = 0x00;
    data_8b[idx++] = 0x00;
    data_8b[idx++] = 0x80;
    data_8b[idx++] = 0x11;
    // IP Check SUM
    data_8b[idx++] = (ip_chk_sum3 >> 8) & 0xFF;
    data_8b[idx++] = (ip_chk_sum3 >> 0) & 0xFF;
    // IP Source
    data_8b[idx++] = ip_src1;
    data_8b[idx++] = ip_src2;
    data_8b[idx++] = ip_src3;
    data_8b[idx++] = ip_src4;
    // IP Destination
    data_8b[idx++] = ip_dst1;
    data_8b[idx++] = ip_dst2;
    data_8b[idx++] = ip_dst3;
    data_8b[idx++] = ip_dst4;
    // UDP header
    data_8b[idx++] = (src_port >> 8) & 0xFF;
    data_8b[idx++] = (src_port >> 0) & 0xFF;
    data_8b[idx++] = (dest_port >> 8) & 0xFF;
    data_8b[idx++] = (dest_port >> 0) & 0xFF;
    data_8b[idx++] = (udpLen >> 8) & 0xFF;
    data_8b[idx++] = (udpLen >> 0) & 0xFF;
    data_8b[idx++] = (udp_chksum >> 8) & 0xFF;
    data_8b[idx++] = (udp_chksum >> 0) & 0xFF;
    // UDP payload
    for (i = 0; i < payloadLength; i++)
    {
        data_8b[idx++] = udp_payload[i];
    }

    //==========================================================================
    // FCS Calc
    //==========================================================================
    uint32_t crc = 0xffffffff;
    for (i = 8; i < idx; i++)
    {
        crc = (crc >> 8) ^ crc_table[(crc ^ data_8b[i]) & 0xFF];
    }
    crc ^= 0xffffffff;

    data_8b[idx++] = (crc >> 0) & 0xFF;
    data_8b[idx++] = (crc >> 8) & 0xFF;
    data_8b[idx++] = (crc >> 16) & 0xFF;
    data_8b[idx++] = (crc >> 24) & 0xFF;

    //==========================================================================
    // Manchester Encoder
    //==========================================================================
    for (i = 0; i < udp_buf_size; i++)
    {
        buf[i] = tbl_manchester[data_8b[i]];
    }
    // TP_IDL
    buf[i] = 0x00000AAA;

    free(data_8b);
}

void eth_send_nlp()
{
    uint32_t data_32b;
    // 最初にNLPを送って対向機器に10BASE-T半二重であることを認識させる
    for (uint32_t nlp = 0, lp = 0; nlp < 200;)
    {
        // リンクパルスは16ms毎
        // ループ周期は0.8us
        // 16ms / 0.8us = 32000
        if (++lp == 20000)
        {
            lp = 0;
            nlp++;
            data_32b = 0x0000000A; // High(パルス幅100ns)
        }
        else
        {
            data_32b = 0x00000000; // IDLE
        }
        ser_10base_t_tx_10b(pio_ser_wr, 0, data_32b);
    }
}

void eth_transmit_udp(uint8_t udp_payload[], uint16_t payloadLength)
{
    uint16_t udp_buf_size = payloadLength + 54;
    uint32_t *tx_buf_udp = (uint32_t *)calloc(udp_buf_size + 1, sizeof(uint32_t));

    udp_packet_gen_10base(tx_buf_udp, udp_payload, payloadLength);
    for (uint32_t i = 0; i < udp_buf_size + 1; i++)
    {
        ser_10base_t_tx_10b(pio_ser_wr, 0, tx_buf_udp[i]);
    }

    free(tx_buf_udp);
}

void eth_generate_mac()
{
    pico_unique_board_id_t pico_uid;
    pico_get_unique_board_id(&pico_uid);

    uint32_t lastBytes = 0;
    lastBytes |= pico_uid.id[5];
    lastBytes |= (pico_uid.id[6] << 8);
    lastBytes |= (pico_uid.id[7] << 16);

    mac_src |= lastBytes;
}

void eth_init()
{
    eth_generate_mac();
    udp_init();

    // 10BASE-T Serializer PIO init
    // sideset を使う都合上、GPIOピンは連番である必要がある。16,17を使用する。
    uint offset = pio_add_program(pio_ser_wr, &ser_10base_t_program);
    ser_10base_t_program_init(pio_ser_wr, 0, offset, 16);

    eth_send_nlp();
}

static struct repeating_timer nlp_timer;

typedef struct
{
    uint8_t ticks;
    bool transmitting;
} nlp_state_t;

static bool nlp_timer_callback(struct repeating_timer *t)
{
    nlp_state_t *state = t->user_data;
    if (state->transmitting == false)
        state->ticks++;
    return true;
}

void core1_entry()
{
    eth_init();

    static nlp_state_t state;
    state.ticks = 0;
    state.transmitting = false;

    alarm_pool_t *core1_pool = alarm_pool_create(0, 1);
    alarm_pool_add_repeating_timer_ms(core1_pool, 5, nlp_timer_callback, &state, &nlp_timer);

    while (1)
    {
        uint8_t payload[MAX_UDP_PAYLOAD_SIZE] = {0};
        if (queue_try_remove(&udp_payload_queue, payload))
        {
            uint16_t payloadLength = strlen(payload);
            state.transmitting = true;
            eth_transmit_udp(payload, payloadLength);
            state.transmitting = false;
            state.ticks = 0;
        }
        else if (state.ticks >= 4)
        {
            state.transmitting = true;
            eth_send_nlp();
            state.transmitting = false;
            state.ticks = 0;
        }
    }
}

void eth_core_start()
{
    queue_init(&udp_payload_queue, MAX_UDP_PAYLOAD_SIZE, 1);

    multicore_reset_core1();
    multicore_fifo_drain();
    multicore_launch_core1(core1_entry);
}

void eth_set_ip(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4)
{
    ip_src1 = ip1;
    ip_src2 = ip2;
    ip_src3 = ip3;
    ip_src4 = ip4;
}

void eth_set_dest(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4)
{
    ip_dst1 = ip1;
    ip_dst2 = ip2;
    ip_dst3 = ip3;
    ip_dst4 = ip4;
}

void udp_printf(const char *format, ...)
{
    uint8_t payload[MAX_UDP_PAYLOAD_SIZE] = {0};
    va_list args;
    va_start(args, format);
    vsprintf(payload, format, args);
    queue_add_blocking(&udp_payload_queue, payload);
    va_end(args);
}
