#include "cheetah_dpdk.h"

void cheetah_packet_ether_addr_populate(struct ether_addr *addr, uint8_t a,
                                        uint8_t b, uint8_t c, uint8_t d,
                                        uint8_t e, uint8_t f) {
  addr->addr_bytes[0] = a;
  addr->addr_bytes[1] = b;
  addr->addr_bytes[2] = c;
  addr->addr_bytes[3] = d;
  addr->addr_bytes[4] = e;
  addr->addr_bytes[5] = f;
}

void cheetah_prepend_packet_payload(const struct cheetah_packet *const pkt,
                                    struct rte_mbuf *mbuf) {
  cheetah_header_field_t *row_id;

#ifdef CHEETAH_USE_64_BIT_VALUES
  cheetah_header_field_large_t *value;
#else
  cheetah_header_field_t *value;
#endif /* CHEETAH_USE_64_BIT_VALUES */

#ifdef CHEETAH_USE_KEY_AT_WORKER
  cheetah_header_field_t *key;
#endif /* CHEETAH_USE_KEY_AT_WORKER */

  cheetah_header_field_small_t *flow_id;

  RTE_ASSERT(pkt != NULL);
  RTE_ASSERT(state != NULL);

#ifdef CHEETAH_USE_64_BIT_VALUES
  value = (cheetah_header_field_large_t *)rte_pktmbuf_prepend(
      mbuf, sizeof(cheetah_header_field_large_t));
#else
  value = (cheetah_header_field_t *)rte_pktmbuf_prepend(
      mbuf, sizeof(cheetah_header_field_t));
#endif /* CHEETAH_USE_64_BIT_VALUES */

  RTE_ASSERT(value != NULL);

  *value = pkt->value;

#ifdef CHEETAH_USE_KEY_AT_WORKER
  key = (cheetah_header_field_t *)rte_pktmbuf_prepend(
      mbuf, sizeof(cheetah_header_field_t));
  RTE_ASSERT(key != NULL);

  *key = pkt->key;
#endif /* CHEETAH_USE_KEY_AT_WORKER */

  row_id = (cheetah_header_field_t *)rte_pktmbuf_prepend(
      mbuf, sizeof(cheetah_header_field_t));
  RTE_ASSERT(row_id != NULL);

  *row_id = pkt->row_id;

  /* temporary hack */
  flow_id = (uint16_t *)rte_pktmbuf_prepend(mbuf, sizeof(uint16_t));
  RTE_ASSERT(flow_id != NULL);

  *flow_id = pkt->flow_id;
}

void cheetah_prepend_ethernet_header(
    const struct cheetah_connection_state *const state, struct rte_mbuf *mbuf) {
  struct ether_hdr *hdr;

  hdr = (struct ether_hdr *)rte_pktmbuf_prepend(mbuf, sizeof(struct ether_hdr));
  if (hdr == NULL)
    rte_exit(EXIT_FAILURE, "Could not append header to packet pool.");

  ether_addr_copy(&state->destination_mac, &hdr->d_addr);
  ether_addr_copy(&state->mac_address, &hdr->s_addr);

  hdr->ether_type = rte_cpu_to_be_16(ETHER_TYPE_IPv4);
}

void cheetah_unpack_ethernet_header(struct rte_mbuf *mbuf) {
  cheetah_unused_var(mbuf);
  RTE_ASSERT(rte_pktmbuf_adj(mbuf, (uint16_t)sizeof(struct ether_hdr)) != NULL);
}

void cheetah_unpack_ip_header(struct rte_mbuf *mbuf) {
  cheetah_unused_var(mbuf);
  RTE_ASSERT(RTE_ETH_IS_IPV4_HDR(mbuf->packet_type));
  RTE_ASSERT(mbuf->next_proto_id == 0x11); /* UDP Check */

  RTE_ASSERT(rte_pktmbuf_adj(mbuf, (uint16_t)sizeof(struct ipv4_hdr)) != NULL);
}

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
) {
  uint16_t total_header_size;

  total_header_size = sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) +
                      sizeof(struct udp_hdr);

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  *flow_id = *(rte_pktmbuf_mtod_offset(mbuf, cheetah_header_field_small_t *,
                                       total_header_size));

  *flow_id = rte_be_to_cpu_16(*flow_id);
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

  total_header_size += sizeof(cheetah_header_field_small_t);

  *row_id = *(rte_pktmbuf_mtod_offset(mbuf, cheetah_header_field_t *,
                                      total_header_size));

  *row_id = rte_be_to_cpu_32(*row_id);

  total_header_size += sizeof(cheetah_header_field_t);

#ifdef CHEETAH_USE_KEY_AT_WORKER
  *key = *(rte_pktmbuf_mtod_offset(mbuf, cheetah_header_field_t *,
                                   total_header_size));
  *key = rte_be_to_cpu_32(*key);
  total_header_size += sizeof(cheetah_header_field_t);
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_64_BIT_VALUES
  *value = *(rte_pktmbuf_mtod_offset(mbuf, cheetah_header_field_large_t *,
                                     total_header_size));

  *value = rte_be_to_cpu_64(*value);
#else
  *value = *(rte_pktmbuf_mtod_offset(mbuf, cheetah_header_field_t *,
                                     total_header_size));

  *value = rte_be_to_cpu_32(*value);
#endif /* CHEETAH_USE_64_BIT_VALUES */
}

void cheetah_unpack_udp_header(struct rte_mbuf *mbuf) {
  cheetah_unused_var(mbuf);
  RTE_ASSERT(rte_pktmbuf_adj(mbuf, (uint16_t)sizeof(struct udp_hdr)) != NULL);
}

void cheetah_prepend_ip_header(
    const struct cheetah_connection_state *const state, struct rte_mbuf *mbuf) {
  struct ipv4_hdr *hdr;

  hdr = (struct ipv4_hdr *)rte_pktmbuf_prepend(mbuf, sizeof(struct ipv4_hdr));
  RTE_ASSERT(hdr != NULL);

  /* Version (4 bits) | Internal header length (4 bits) */
  hdr->version_ihl = 0x40 | 0x05;

  hdr->type_of_service = 0x0;
  hdr->total_length = rte_cpu_to_be_16(CHEETAH_DATAGRAM_LENGTH);
  hdr->packet_id = rte_cpu_to_be_16(1);
  hdr->fragment_offset = rte_cpu_to_be_16(0);
  hdr->time_to_live = 64;

  /* UDP = 0x11 */
  hdr->next_proto_id = IPPROTO_UDP;

  /*
  hdr->src_addr = state->source_ip;
  hdr->dst_addr = state->destination_ip;
  */

  /* IP Addr Hack */

  hdr->src_addr = rte_cpu_to_be_32(CHEETAH_WORKER_DPDK_IP);
  hdr->dst_addr = rte_cpu_to_be_32(CHEETAH_MASTER_DPDK_IP);

  /* TODO: remove this and fix ip issue above */
  cheetah_unused_var(state);

  hdr->hdr_checksum = rte_ipv4_cksum(hdr);
}

void cheetah_prepend_udp_header(
    const struct cheetah_connection_state *const state, struct rte_mbuf *mbuf) {
  struct udp_hdr *hdr;

  hdr = (struct udp_hdr *)rte_pktmbuf_prepend(mbuf, sizeof(struct udp_hdr));
  RTE_ASSERT(hdr != NULL);

  hdr->src_port = rte_cpu_to_be_16(state->udp_source_port);
  hdr->dst_port = rte_cpu_to_be_16(state->udp_destination_port);

  hdr->dgram_len = rte_cpu_to_be_16(CHEETAH_UDP_DATAGRAM_LENGTH);
  hdr->dgram_cksum = rte_cpu_to_be_16(0);
}
