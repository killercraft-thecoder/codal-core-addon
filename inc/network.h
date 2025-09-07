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

// -----------------------------------------------------------------------------
// Ethernet II frame header
// -----------------------------------------------------------------------------

/**
 * @brief Ethernet II frame header (14 bytes).
 *
 * This struct is packed to match the exact on‑wire layout.
 * Use with care on MCUs that fault on unaligned access.
 */
typedef struct NET_PACKED {
    uint8_t  dest_mac[6];   /**< Destination MAC address */
    uint8_t  src_mac[6];    /**< Source MAC address */
    uint16_t ethertype;     /**< EtherType field (e.g., 0x0800 = IPv4, 0x86DD = IPv6) */
} eth_header_t;

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
  _Static_assert(sizeof(eth_header_t) == 14, "Ethernet header must be 14 bytes");
#endif

// Common EtherType values
#define ETH_TYPE_IPV4  0x0800
#define ETH_TYPE_ARP   0x0806
#define ETH_TYPE_IPV6  0x86DD

// -----------------------------------------------------------------------------
// UDP datagram header
// -----------------------------------------------------------------------------

/**
 * @brief UDP datagram header (8 bytes).
 *
 * Packed to match the exact on‑wire layout.
 * All multi‑byte fields are in network byte order.
 */
typedef struct NET_PACKED {
    uint16_t src_port;   /**< Source port */
    uint16_t dest_port;  /**< Destination port */
    uint16_t length;     /**< Length of UDP header + data */
    uint16_t checksum;   /**< UDP checksum (0 = optional for IPv4) */
} udp_header_t;

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
  _Static_assert(sizeof(udp_header_t) == 8, "UDP header must be 8 bytes");
#endif

// -----------------------------------------------------------------------------
// Ethernet I / II common MAC header
// -----------------------------------------------------------------------------
typedef struct NET_PACKED {
    uint8_t  dest_mac[6];   /**< Destination MAC address */
    uint8_t  src_mac[6];    /**< Source MAC address */
    uint16_t type_length;   /**< EtherType (II) or length (802.3 / I) */
} eth_mac_header_t;

_Static_assert(sizeof(eth_mac_header_t) == 14, "Ethernet MAC header must be 14 bytes");

// -----------------------------------------------------------------------------
// Ethernet II interpretation
// -----------------------------------------------------------------------------
typedef struct NET_PACKED {
    uint8_t  dest_mac[6];
    uint8_t  src_mac[6];
    uint16_t ethertype;     /**< Protocol type: e.g., 0x0800 = IPv4, 0x86DD = IPv6 */
} eth2_header_t;

#define ETH_TYPE_IPV4  0x0800
#define ETH_TYPE_ARP   0x0806
#define ETH_TYPE_IPV6  0x86DD

// -----------------------------------------------------------------------------
// IEEE 802.3 interpretation (Ethernet I and later 802.3)
// -----------------------------------------------------------------------------
typedef struct NET_PACKED {
    uint8_t  dest_mac[6];
    uint8_t  src_mac[6];
    uint16_t length;        /**< Payload length in bytes (≤ 1500) */
    // Followed by LLC/SNAP header
} eth8023_header_t;

_Static_assert(sizeof(eth8023_header_t) == 14, "802.3 header must be 14 bytes");

// LLC header (802.2)
typedef struct NET_PACKED {
    uint8_t dsap;           /**< Destination Service Access Point */
    uint8_t ssap;           /**< Source Service Access Point */
    uint8_t control;        /**< Control field */
} llc_header_t;

// SNAP header (if dsap/ssap == 0xAA)
typedef struct NET_PACKED {
    uint8_t  oui[3];        /**< Organizationally Unique Identifier */
    uint16_t protocol_id;   /**< Protocol type (same values as EtherType) */
} snap_header_t;

#ifdef __cplusplus
}
#endif
