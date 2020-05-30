#ifndef FAST_PACKET_STREAM_H
#define FAST_PACKET_STREAM_H

struct cheetah_fast_packet_stream {
  struct rte_mbuf **mbuf_burst_arr;

  cheetah_header_field_t **payload_row_id_arr;

#if defined(CHEETAH_USE_64_BIT_VALUES) || defined(CHEETAH_USE_STRING_VALUES)
  cheetah_header_field_large_t **payload_value_arr;
#else  /* not string or 64-bit */
  cheetah_header_field_t **payload_value_arr;
#endif /* CHEETAH_USE_64_BIT_VALUES */

#ifdef CHEETAH_USE_HASH_AT_WORKER
  cheetah_header_field_t **payload_hash_arr;
#endif /* CHEETAH_USE_HASH_AT_WORKER */

#ifdef CHEETAH_USE_KEY_AT_WORKER
  cheetah_header_field_t **payload_key_arr;
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  cheetah_header_field_small_t **payload_flow_id_arr;
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

  size_t packet_count;
};

struct cheetah_fast_packet_stream *cheetah_fast_packet_stream_create(
    const struct cheetah_connection_state *const state,
    const size_t packet_count);

void cheetah_fast_packet_stream_send(
    const struct cheetah_fast_packet_stream *const stream,
    const struct cheetah_connection_state *const state);

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
    const cheetah_header_field_t row_id_new);

void cheetah_fast_packet_stream_free(struct cheetah_fast_packet_stream *stream);

#endif /* FAST_PACKET_STREAM_H */
