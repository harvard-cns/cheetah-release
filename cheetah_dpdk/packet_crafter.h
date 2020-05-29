#ifndef CHEETAH_PACKET_CRAFTER_H
#define CHEETAH_PACKET_CRAFTER_H

void cheetah_packet_ether_addr_populate(struct ether_addr *addr, uint8_t a,
                                        uint8_t b, uint8_t c, uint8_t d,
                                        uint8_t e, uint8_t f);

void cheetah_prepend_ethernet_header(
    const struct cheetah_connection_state *const state, struct rte_mbuf *mbuf);

void cheetah_prepend_packet_payload(const struct cheetah_packet *const pkt,
                                    struct rte_mbuf *mbuf);

void cheetah_prepend_ip_header(
    const struct cheetah_connection_state *const state, struct rte_mbuf *mbuf);

void cheetah_prepend_udp_header(
    const struct cheetah_connection_state *const state, struct rte_mbuf *mbuf);

void cheetah_unpack_ethernet_header(struct rte_mbuf *mbuf);

void cheetah_unpack_ip_header(struct rte_mbuf *mbuf);

void cheetah_unpack_udp_header(struct rte_mbuf *mbuf);

void cheetah_unpack_payload(struct rte_mbuf *mbuf,
                            cheetah_header_field_t *row_id,
#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
                            cheetah_header_field_small_t *flow_id,
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */
#ifdef CHEETAH_USE_KEY_AT_WORKER
                            cheetah_header_field_t *key,
#endif /* CHEETAH_USE_KEY_AT_WORKER */
#if defined(CHEETAH_USE_64_BIT_VALUES) || defined(CHEETAH_USE_STRING_VALUES)
                            cheetah_header_field_large_t *value
#else
                            cheetah_header_field_t *value
#endif /* CHEETAH_USE_64_BIT_VALUES */
);

#endif /* CHEETAH_PACKET_CRAFTER_H */
