#pragma once
/**
 * @file network.h
 * @brief Basic IP address and networking definitions for CODAL-style addons.
 *
 * This header defines IPv4/IPv6 address structures, constants, and helper
 * macros that are compatible with real IP networking. It is safe to use in
 * embedded environments where devices share the same physical link.
 *
 * @note This file does not implement a TCP/IP stack. It is intended as a
 *       foundation for network-aware drivers or SVC-based network services.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Packing helpers
#if defined(__GNUC__) || defined(__clang__)
  #define NET_PACKED __attribute__((packed))
  #define NET_ALIGNED(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
  #define NET_PACKED
  #define NET_ALIGNED(x)
  #pragma pack(push, 1)
#else
  #define NET_PACKED
  #define NET_ALIGNED(x)
#endif

// -----------------------------------------------------------------------------
// IPv4 definitions
// -----------------------------------------------------------------------------

/** IPv4 address in network byte order */
typedef struct NET_PACKED {
    uint8_t bytes[4];
} ipv4_addr_t;

/** Common IPv4 addresses */
#define IPV4_ADDR_ANY        ((ipv4_addr_t){{0,0,0,0}})
#define IPV4_ADDR_BROADCAST  ((ipv4_addr_t){{255,255,255,255}})
#define IPV4_ADDR_LOOPBACK   ((ipv4_addr_t){{127,0,0,1}})

/** Construct an IPv4 address from octets */
#define IPV4_ADDR(a,b,c,d)   ((ipv4_addr_t){{(a),(b),(c),(d)}})

/** Convert an IPv4 address to a 32-bit integer (network byte order) */
static inline uint32_t ipv4_to_u32(ipv4_addr_t addr) {
    return ((uint32_t)addr.bytes[0] << 24) |
           ((uint32_t)addr.bytes[1] << 16) |
           ((uint32_t)addr.bytes[2] << 8)  |
           ((uint32_t)addr.bytes[3]);
}

/** Convert a 32-bit integer (network byte order) to an IPv4 address */
static inline ipv4_addr_t u32_to_ipv4(uint32_t val) {
    return (ipv4_addr_t){{
        (uint8_t)(val >> 24),
        (uint8_t)(val >> 16),
        (uint8_t)(val >> 8),
        (uint8_t)(val)
    }};
}

// -----------------------------------------------------------------------------
// IPv6 definitions
// -----------------------------------------------------------------------------

/** IPv6 address in network byte order */
typedef struct NET_PACKED {
    uint8_t bytes[16];
} ipv6_addr_t;

/** Common IPv6 addresses */
#define IPV6_ADDR_ANY        ((ipv6_addr_t){{0}})
#define IPV6_ADDR_LOOPBACK   ((ipv6_addr_t){{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}})

/** Construct an IPv6 address from 16 bytes */
#define IPV6_ADDR( \
    b0,b1,b2,b3,b4,b5,b6,b7, \
    b8,b9,b10,b11,b12,b13,b14,b15) \
    ((ipv6_addr_t){{ \
        b0,b1,b2,b3,b4,b5,b6,b7, \
        b8,b9,b10,b11,b12,b13,b14,b15 \
    }})

// -----------------------------------------------------------------------------
// Port numbers
// -----------------------------------------------------------------------------

/** Well-known ports */
#define PORT_HTTP   80
#define PORT_HTTPS  443
#define PORT_DNS    53
#define PORT_NTP    123
#define PORT_MQTT   1883

// -----------------------------------------------------------------------------
// Utility
// -----------------------------------------------------------------------------

/**
 * @brief Compare two IPv4 addresses for equality.
 */
static inline int ipv4_equal(ipv4_addr_t a, ipv4_addr_t b) {
    return (a.bytes[0] == b.bytes[0] &&
            a.bytes[1] == b.bytes[1] &&
            a.bytes[2] == b.bytes[2] &&
            a.bytes[3] == b.bytes[3]);
}

/**
 * @brief Compare two IPv6 addresses for equality.
 */
static inline int ipv6_equal(ipv6_addr_t a, ipv6_addr_t b) {
    for (int i = 0; i < 16; i++) {
        if (a.bytes[i] != b.bytes[i]) return 0;
    }
    return 1;
}

/**
 * @brief Generic network packet structure.
 *
 * This structure is designed to be compatible with real IPv4/IPv6 + TCP/UDP
 * framing so that, if connected to a real network stack, the data will be
 * interpreted correctly by other devices (including web browsers).
 *
 * @note This is a *container* for packet metadata and payload. It does not
 *       implement any protocol logic — that’s up to the network stack.
 */
typedef struct NET_PACKED {
    /** Source IP address (IPv4 or IPv6) */
    union {
        ipv4_addr_t v4;
        ipv6_addr_t v6;
    } src;

    /** Destination IP address (IPv4 or IPv6) */
    union {
        ipv4_addr_t v4;
        ipv6_addr_t v6;
    } dest;

    /** Source port (TCP/UDP) in host byte order */
    uint16_t srcPort;

    /** Destination port (TCP/UDP) in host byte order */
    uint16_t destPort;

    /** Protocol number (e.g., 6 = TCP, 17 = UDP, per IANA) */
    uint8_t protocol;

    /** Length of valid data in payload[] */
    uint16_t length;

    /** Packet payload (raw bytes) */
    uint8_t payload[1500]; // MTU-sized buffer (Ethernet default)

} net_packet_t;

#ifdef __cplusplus
}
#endif
