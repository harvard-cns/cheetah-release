#include "cheetah_dpdk.h"

void cheetah_fast_packet_stream_send(
    const struct cheetah_fast_packet_stream *const stream,
    const struct cheetah_connection_state *const state) {

  struct rte_mbuf **mbuf_burst_arr_clone;
  uint16_t packets_sent;
  size_t packets_left;
  register size_t i;

  mbuf_burst_arr_clone =
      malloc(sizeof(struct rte_mbuf *) * stream->packet_count);

  for (i = 0; i < stream->packet_count; i++) {
    mbuf_burst_arr_clone[i] =
        rte_pktmbuf_clone(stream->mbuf_burst_arr[i], state->tx_packet_pool);

    RTE_ASSERT(mbuf_burst_arr_clone[i] != NULL);
  }

  packets_left = stream->packet_count;

  do {
    packets_sent = rte_eth_tx_burst(state->portid, CHEETAH_TX_QUEUE_ID,
                                    mbuf_burst_arr_clone, packets_left);
    packets_left -= packets_sent;
    mbuf_burst_arr_clone += packets_sent;
  } while (packets_left != 0);
}

void cheetah_fast_packet_stream_modify(
    const struct cheetah_fast_packet_stream *const stream,
    const size_t packet_index,
#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
    const cheetah_header_field_small_t flow_id_new,
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */
#ifdef CHEETAH_USE_KEY_AT_WORKER
    const cheetah_header_field_t key_new,
#endif /* CHEETAH_USE_KEY_AT_WORKER */
#if defined(CHEETAH_USE_64_BIT_VALUES) || defined(CHEETAH_USE_STRING_VALUES)
    const cheetah_header_field_large_t value_new,
#else
    const cheetah_header_field_t value_new,
#endif /* CHEETAH_USE_64_BIT_VALUES or CHEETAH_USE_STRING_VALUES*/
    const cheetah_header_field_t row_id_new) {

  cheetah_header_field_t *prow_id;

#if defined(CHEETAH_USE_64_BIT_VALUES) || defined(CHEETAH_USE_STRING_VALUES)
  cheetah_header_field_large_t *pvalue;
#else
  cheetah_header_field_t *pvalue;
#endif /* CHEETAH_USE_64_BIT_VALUES */

#ifdef CHEETAH_USE_KEY_AT_WORKER
  cheetah_header_field_t *pkey;
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  cheetah_header_field_small_t *pflow_id;
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

  RTE_ASSERT(stream != NULL);
  RTE_ASSERT(packet_index < stream->packet_count);

  prow_id = stream->payload_row_id_arr[packet_index];
  pvalue = stream->payload_value_arr[packet_index];

#ifdef CHEETAH_USE_KEY_AT_WORKER
  pkey = stream->payload_key_arr[packet_index];
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  pflow_id = stream->payload_flow_id_arr[packet_index];
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

  *prow_id = rte_cpu_to_be_32(row_id_new);

#if defined(CHEETAH_USE_64_BIT_VALUES) || defined(CHEETAH_USE_STRING_VALUES)
  *pvalue = rte_cpu_to_be_64(value_new);
#else
  *pvalue = rte_cpu_to_be_32(value_new);
#endif /* CHEETAH_USE_64_BIT_VALUES */

#ifdef CHEETAH_USE_KEY_AT_WORKER
  *pkey = rte_cpu_to_be_32(key_new);
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  *pflow_id = rte_cpu_to_be_16(flow_id_new);
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */
}

void cheetah_fast_packet_stream_free(
    struct cheetah_fast_packet_stream *stream) {
  register unsigned int i;

  RTE_ASSERT(stream != NULL);

  for (i = 0; i < stream->packet_count; i++) {
    rte_pktmbuf_free(stream->mbuf_burst_arr[i]);
  }

  free(stream->mbuf_burst_arr);
  free(stream->payload_row_id_arr);
  free(stream->payload_value_arr);

#ifdef CHEETAH_USE_KEY_AT_WORKER
  free(stream->payload_key_arr);
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  free(stream->payload_flow_id_arr);
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

  free(stream);
}

struct cheetah_fast_packet_stream *cheetah_fast_packet_stream_create(
    const struct cheetah_connection_state *const state,
    const size_t packet_count) {
  struct cheetah_fast_packet_stream *stream;
  struct rte_mbuf *mbuf;
  cheetah_header_field_t *row_id;

#if defined(CHEETAH_USE_64_BIT_VALUES) || defined(CHEETAH_USE_STRING_VALUES)
  cheetah_header_field_large_t *value;
#else
  cheetah_header_field_t *value;
#endif /* CHEETAH_USE_64_BIT_VALUES */

#ifdef CHEETAH_USE_KEY_AT_WORKER
  cheetah_header_field_t *key;
#endif /* CHEETAH_USE_KEY_AT_WORKER */

  cheetah_header_field_small_t *flow_id;

  register unsigned int i;

  stream = malloc(sizeof(struct cheetah_fast_packet_stream));
  RTE_ASSERT(stream != NULL);

  RTE_ASSERT(packet_count != 0);
  stream->packet_count = packet_count;

  stream->mbuf_burst_arr = malloc(sizeof(struct rte_mbuf *) * packet_count);
  RTE_ASSERT(stream->mbuf_burst_arr != NULL);

  stream->payload_row_id_arr =
      malloc(sizeof(cheetah_header_field_t *) * packet_count);
  RTE_ASSERT(stream->payload_row_id_arr != NULL);

#if defined(CHEETAH_USE_64_BIT_VALUES) || defined(CHEETAH_USE_STRING_VALUES)
  stream->payload_value_arr =
      malloc(sizeof(cheetah_header_field_large_t *) * packet_count);
#else
  stream->payload_value_arr =
      malloc(sizeof(cheetah_header_field_t *) * packet_count);
#endif /* CHEETAH_USE_64_BIT_VALUES */

  RTE_ASSERT(stream->payload_value_arr != NULL);

#ifdef CHEETAH_USE_KEY_AT_WORKER
  stream->payload_key_arr =
      malloc(sizeof(cheetah_header_field_t *) * packet_count);

  RTE_ASSERT(stream->payload_key_arr != NULL);
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  stream->payload_flow_id_arr =
      malloc(sizeof(cheetah_header_field_small_t *) * packet_count);

  RTE_ASSERT(stream->payload_flow_id_arr != NULL);
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

  for (i = 0; i < packet_count; i++) {
    mbuf = rte_pktmbuf_alloc(state->tx_packet_pool);
    RTE_ASSERT(mbuf != NULL);

    stream->mbuf_burst_arr[i] = mbuf;

#if defined(CHEETAH_USE_64_BIT_VALUES) || defined(CHEETAH_USE_STRING_VALUES)
    value = (cheetah_header_field_large_t *)rte_pktmbuf_prepend(
        mbuf, sizeof(cheetah_header_field_large_t));
#else
    value = (cheetah_header_field_t *)rte_pktmbuf_prepend(
        mbuf, sizeof(cheetah_header_field_t));
#endif /* CHEETAH_USE_64_BIT_VALUES */

#ifdef CHEETAH_USE_KEY_AT_WORKER
    key = (cheetah_header_field_t *)rte_pktmbuf_prepend(
        mbuf, sizeof(cheetah_header_field_t));
#endif /* CHEETAH_USE_KEY_AT_WORKER */

    row_id = (cheetah_header_field_t *)rte_pktmbuf_prepend(
        mbuf, sizeof(cheetah_header_field_t));

    /* temporary hack for flow_id */
    flow_id = (uint16_t *)rte_pktmbuf_prepend(mbuf, sizeof(uint16_t));

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
    stream->payload_flow_id_arr[i] = flow_id;
#else  /* CHEETAH_USE_FLOW_ID_AT_WORKER */
    *flow_id = rte_cpu_to_be_16(CHEETAH_DEFAULT_FLOW_ID);
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

    stream->payload_row_id_arr[i] = row_id;

#ifdef CHEETAH_USE_KEY_AT_WORKER
    stream->payload_key_arr[i] = key;
#endif /* CHEETAH_USE_KEY_AT_WORKER */

    stream->payload_value_arr[i] = value;

    cheetah_prepend_udp_header(state, mbuf);
    cheetah_prepend_ip_header(state, mbuf);
    cheetah_prepend_ethernet_header(state, mbuf);
  }

  return stream;
}
