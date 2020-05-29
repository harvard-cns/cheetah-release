#ifndef PACKET_STREAM_RECEIVER_H
#define PACKET_STREAM_RECEIVER_H

struct cheetah_packet_stream_receiver {
  struct rte_mbuf **mbuf_burst_arr;

#if defined(CHEETAH_ENABLE_PACKET_MONITORING) ||                               \
    defined(CHEETAH_ENABLE_AGGREGATE_PROCESSING)
  cheetah_header_field_t *payload_row_id_arr;

#if defined(CHEETAH_USE_64_BIT_VALUES) || defined(CHEETAH_USE_STRING_VALUES)
  cheetah_header_field_large_t *payload_value_arr;
#else
  cheetah_header_field_t *payload_value_arr;
#endif /* CHEETAH_USE_64_BIT_VALUES */

#ifdef CHEETAH_USE_KEY_AT_WORKER
  cheetah_header_field_t *payload_key_arr;
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  cheetah_header_field_small_t *payload_flow_id_arr;
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

#endif /* CHEETAH_ENABLE_RECEIVER_PACKET_MONITORING */

  size_t packet_batch_size;
};

void cheetah_packet_stream_receiver_free(
    struct cheetah_packet_stream_receiver *receiver);

uint32_t cheetah_packet_stream_receiver_receive(
    struct cheetah_packet_stream_receiver *const receiver,
    const struct cheetah_connection_state *state);

struct cheetah_packet_stream_receiver *
cheetah_packet_stream_receiver_create(const size_t packet_batch_size);

#endif /* PACKET_STREAM_RECEIVER_H */
