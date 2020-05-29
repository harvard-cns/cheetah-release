#include "cheetah_dpdk.h"

void cheetah_packet_stream_receiver_free(
    struct cheetah_packet_stream_receiver *receiver) {
#ifdef CHEETAH_ENABLE_RECEIVER_PACKET_MONITORING
  free(receiver->payload_value_arr);
  free(receiver->payload_row_id_arr);
#endif /* CHEETAH_ENABLE_RECEIVER_PACKET_MONITORING */

  free(receiver->mbuf_burst_arr);
  free(receiver);
}

struct cheetah_packet_stream_receiver *
cheetah_packet_stream_receiver_create(const size_t packet_batch_size) {
  struct cheetah_packet_stream_receiver *receiver;

  receiver = malloc(sizeof(struct cheetah_packet_stream_receiver));
  RTE_ASSERT(receiver != NULL);

  receiver->packet_batch_size = packet_batch_size;

  receiver->mbuf_burst_arr =
      malloc(sizeof(struct rte_mbuf *) * packet_batch_size);
  RTE_ASSERT(receiver->mbuf_burst_arr != NULL);

#if defined(CHEETAH_ENABLE_PACKET_MONITORING) ||                               \
    defined(CHEETAH_ENABLE_AGGREGATE_PROCESSING)

  /* Commenting out. Only storing last N packets received now */
  /* (better memory useage) */

  /*
  receiver->payload_current_index = 0;

  receiver->payload_row_id_arr = malloc(sizeof(cheetah_header_field_t)
                                              *
  CHEETAH_RECEIVER_MEMORY_ENTRY_COUNT); RTE_ASSERT(receiver->payload_row_id_arr
  != NULL);

  receiver->payload_value_arr = malloc(sizeof(cheetah_header_field_t)
                                              *
  CHEETAH_RECEIVER_MEMORY_ENTRY_COUNT); RTE_ASSERT(receiver->payload_value_arr
  != NULL);
  */

  receiver->payload_row_id_arr =
      malloc(sizeof(cheetah_header_field_t) * packet_batch_size);

  RTE_ASSERT(receiver->payload_row_id_arr != NULL);

#if defined(CHEETAH_USE_64_BIT_VALUES) || defined(CHEETAH_USE_STRING_VALUES)
  receiver->payload_value_arr =
      malloc(sizeof(cheetah_header_field_large_t) * packet_batch_size);
#else
  receiver->payload_value_arr =
      malloc(sizeof(cheetah_header_field_t) * packet_batch_size);
#endif /* CHEETAH_USE_64_BIT_VALUES or CHEETAH_USE_STRING_VALUES */

  RTE_ASSERT(receiver->payload_value_arr != NULL);

#ifdef CHEETAH_USE_KEY_AT_WORKER
  receiver->payload_key_arr =
      malloc(sizeof(cheetah_header_field_t) * packet_batch_size);

  RTE_ASSERT(receiver->payload_key_arr != NULL);
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  receiver->payload_flow_id_arr =
      malloc(sizeof(cheetah_header_field_small_t) * packet_batch_size);

  RTE_ASSERT(receiver->payload_flow_id_arr != NULL);
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

#endif /* CHEETAH_ENABLE_PACKET_MONITORING */

  return receiver;
}

/* #define CHEETAH_DISABLE_PACKET_PARSING */

uint32_t cheetah_packet_stream_receiver_receive(
    struct cheetah_packet_stream_receiver *const receiver,
    const struct cheetah_connection_state *state) {

  uint32_t packets_received;
  register uint32_t i;
  struct rte_mbuf *mbuf;

#if defined(CHEETAH_ENABLE_PACKET_MONITORING) ||                               \
    defined(CHEETAH_ENABLE_AGGREGATE_PROCESSING)
  cheetah_header_field_t row_id;

#ifdef CHEETAH_USE_KEY_AT_WORKER
  cheetah_header_field_t key;
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  cheetah_header_field_small_t flow_id;
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

#if defined(CHEETAH_USE_64_BIT_VALUES) || defined(CHEETAH_USE_STRING_VALUES)
  cheetah_header_field_large_t value;
#else
  cheetah_header_field_t value;
#endif /* CHEETAH_USE_64_BIT_VALUES or CHEETAH_USE_STRING_VALUES*/

#endif /* CHEETAH_ENABLE_RECEIVER_PACKET_MONITORING or                         \
          CHEETAH_ENABLE_AGGREGATE_PROCESSING */
  packets_received = rte_eth_rx_burst(
      state->portid, 0, receiver->mbuf_burst_arr, receiver->packet_batch_size);

  for (i = 0; i < packets_received; i++) {
    mbuf = receiver->mbuf_burst_arr[i];

    /* No longer needed since changed parsing style */
    /*
    cheetah_unpack_ethernet_header(mbuf);
    cheetah_unpack_ip_header(mbuf);
    cheetah_unpack_udp_header(mbuf);
    */

    /*
    cheetah_debug("Here\n");
    cheetah_debug_flush();
    */

    /* Uncomment for monitoring
    rte_pktmbuf_dump(stdout, mbuf, CHEETAH_ETHERNET_FULL_LENGTH);
    fflush(stdout);
    */

#if defined(CHEETAH_ENABLE_PACKET_MONITORING) ||                               \
    defined(CHEETAH_ENABLE_AGGREGATE_PROCESSING)

    cheetah_unpack_payload(mbuf, &row_id,
#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
                           &flow_id,
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */
#ifdef CHEETAH_USE_KEY_AT_WORKER
                           &key,
#endif /* CHEETAH_USE_KEY_AT_WORKER */
                           &value);
    receiver->payload_row_id_arr[i] = row_id;
    receiver->payload_value_arr[i] = value;

#ifdef CHEETAH_USE_KEY_AT_WORKER
    receiver->payload_key_arr[i] = key;
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
    receiver->payload_flow_id_arr[i] = flow_id;
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

#endif /* CHEETAH_ENABLE_PACKET_MONITORING */

    rte_pktmbuf_free(mbuf);
  }

  return packets_received;
}
