/* Author: Muham,ad Tirmazi
 * Harvard - Cloud, Networks & Systems (CNS) Lab
 */

#include "cheetah_dpdk.h"

struct cheetah_packet *
cheetah_packet_create(const cheetah_header_field_t row_id,
                      const cheetah_header_field_t value) {
  struct cheetah_packet *pkt = malloc(sizeof(struct cheetah_packet));
  pkt->row_id = row_id;
  pkt->value = value;
  pkt->flow_id = 1;

  return pkt;
}

void cheetah_packet_free(struct cheetah_packet *pkt) { free(pkt); }

void cheetah_packet_stream_send(
    const struct cheetah_connection_state *const state,
    struct cheetah_packet_stream *const pktstream) {
  int pkt_len;
  int ret;
  struct cheetah_packet_stream *iter_stream;
  struct rte_mbuf **mbuf_burst_arr;
  struct rte_mbuf *mbuf;

  RTE_ASSERT(state != NULL);
  RTE_ASSERT(pktstream != NULL);

  pkt_len = 0;

  mbuf_burst_arr = NULL;

  cheetah_online_debug("Prepending packets...");
  cheetah_online_debug_flush();

  for (iter_stream = pktstream; iter_stream != NULL;
       iter_stream = iter_stream->next) {
    if (pkt_len == 0)
      mbuf_burst_arr = malloc(sizeof(struct rte_mbuf *));
    else
      mbuf_burst_arr =
          realloc(mbuf_burst_arr, (pkt_len + 1) * sizeof(struct rte_mbuf *));

    RTE_ASSERT(mbuf_burst_arr != NULL);

    mbuf_burst_arr[pkt_len] = rte_pktmbuf_alloc(state->tx_packet_pool);
    RTE_ASSERT(mbuf_burst_arr[pkt_len] != NULL);

    mbuf = mbuf_burst_arr[pkt_len];

    cheetah_prepend_packet_payload(iter_stream->packet, mbuf);
    cheetah_prepend_udp_header(state, mbuf);
    cheetah_prepend_ip_header(state, mbuf);
    cheetah_prepend_ethernet_header(state, mbuf);

    pkt_len++;
  }

  cheetah_online_debug("Success!\n");
  cheetah_online_debug_flush();

  ret = rte_eth_tx_burst(state->portid, CHEETAH_TX_QUEUE_ID, mbuf_burst_arr,
                         pkt_len);

  while (ret < pkt_len) {
    rte_pktmbuf_free(mbuf_burst_arr[ret]);
    ret++;
  }

  free(mbuf_burst_arr);
}

struct cheetah_packet_stream *cheetah_packet_stream_create(void) {
  struct cheetah_packet_stream *stream;

  stream = malloc(sizeof(struct cheetah_packet_stream));
  stream->packet = NULL;
  stream->next = NULL;

  return stream;
}

struct cheetah_packet_stream *
cheetah_packet_stream_add(struct cheetah_packet_stream *const stream,
                          const cheetah_header_field_t row_id,
                          const cheetah_header_field_t value) {

  cheetah_online_debug("Adding packet to stream with value: %d, %d...", row_id,
                       value);
  cheetah_online_debug_flush();

  RTE_ASSERT(state->next == NULL);

  if (stream->packet == NULL) {
    stream->packet = cheetah_packet_create(row_id, value);

    cheetah_online_debug("Success!\n");
    cheetah_online_debug_flush();

    return stream;
  } else {
    stream->next = cheetah_packet_stream_create();
    stream->next->packet = cheetah_packet_create(row_id, value);

    cheetah_online_debug("Success!\n");
    cheetah_online_debug_flush();

    return stream->next;
  }
}

/* int cheetah_packet_send(struct cheetah_connection_state const *state) { */
