#include "cheetah_dpdk.h"
#include <assert.h>

/* #define CHEETAH_DISABLE_DPDK */
#define CHEETAH_CONTROL_FLOW_MASTER_DEBUG
/* #define CHEETAH_CONTROL_FLOW_WORKER_DEBUG */

int cheetah_control_flow_master(int argc, char **const argv) {
#ifdef CHEETAH_DISABLE_DPDK
  cheetah_unused_var(argc);
  cheetah_unused_var(argv);
#endif

#ifndef CHEETAH_DISABLE_DPDK
  struct cheetah_connection_state *state;
  struct cheetah_packet_stream_receiver *receiver;
  struct ether_addr dest_addr;
  struct cheetah_benchmarker benchmarker;
  uint64_t packets_received;
  uint64_t packets_received_current_batch;
  uint8_t offloaded;
  uint8_t started;
  int worker_acks_left;

  /* purely to remove warnings */
  worker_acks_left = 0;
  cheetah_unused_var(worker_acks_left);

  cheetah_debug("DPDK is enabled\n");
  cheetah_debug_flush();

  char current_query_type[] = CHEETAH_CONTROL_MESSAGE_QUERY_JOIN;

#ifdef CHEETAH_ENABLE_AGGREGATE_PROCESSING
  /* Pointers to all query aggregate processing states.
   * Hopefully the compiler will optimize out the unnecessary ones */
  struct cheetah_aggregate_processing_distinct_state *distinct_state;
  struct cheetah_aggregate_processing_topn_state *topn_state;
  struct cheetah_aggregate_processing_groupby_state *groupby_state;
  struct cheetah_join_state *join_state;

  register uint64_t i;
  uint64_t update_count = 0;

  distinct_state = NULL;
  topn_state = NULL;
  groupby_state = NULL;
  join_state = cheetah_join_init(100000, 40000);

  /* Suppress warnings for the above states */
  cheetah_unused_var(distinct_state);
  cheetah_unused_var(topn_state);
  cheetah_unused_var(groupby_state);

#endif /* CHEETAH_ENABLE_AGGREGATE_PROCESSING */

  cheetah_init(argc, argv);
  cheetah_packet_ether_addr_populate(&dest_addr, CHEETAH_WORKER_MAC_1,
                                     CHEETAH_WORKER_MAC_2, CHEETAH_WORKER_MAC_3,
                                     CHEETAH_WORKER_MAC_4, CHEETAH_WORKER_MAC_5,
                                     CHEETAH_WORKER_MAC_6);

#ifdef CHEETAH_ENABLE_RESOURCE_LIMITS
  cheetah_debug("Limiting resources...");
  cheetah_debug_flush();

  cheetah_set_memory_limit(CHEETAH_WORKER_MEMORY_LIMIT);
  cheetah_set_cpu_core_limit(CHEETAH_WORKER_CORE_LIMIT);

  cheetah_debug("Success!\n");
  cheetah_debug_flush();
#endif /* CHEETAH_ENABLE_RESOURCE_LIMITS */

  state = cheetah_connection_state_create(0, dest_addr);
  receiver = cheetah_packet_stream_receiver_create(RECEIVER_BATCH_SIZE);
#endif /* Not CHEETAH_DISABLE_DPDK */

#ifndef CHEETAH_DISABLE_DPDK
  packets_received = 0;
  offloaded = 0;
  started = 0;
  worker_acks_left = CHEETAH_WORKER_COUNT;

#endif /* Not CHEETAH_DISABLE_DPDK */

#ifndef CHEETAH_DISABLE_DPDK
  while (!cheetah_global_force_quit) {
    packets_received_current_batch =
        cheetah_packet_stream_receiver_receive(receiver, state);

    packets_received += packets_received_current_batch;

#ifdef CHEETAH_ENABLE_AGGREGATE_PROCESSING
    for (i = 0; i < packets_received_current_batch; i++) {
      if (strncmp(current_query_type, CHEETAH_CONTROL_MESSAGE_QUERY_GROUPBY,
                  CHEETAH_CONTROL_MESSAGE_MAX_SIZE) == 0) {

        cheetah_aggregate_processing_groupby_update(
            groupby_state, receiver->payload_key_arr[i],
            receiver->payload_value_arr[i]);
      }

      if (strncmp(current_query_type, CHEETAH_CONTROL_MESSAGE_QUERY_DISTINCT,
                  CHEETAH_CONTROL_MESSAGE_MAX_SIZE) == 0) {
        cheetah_aggregate_processing_distinct_update(
            distinct_state, receiver->payload_value_arr[i]);
      }

      if (strncmp(current_query_type, CHEETAH_CONTROL_MESSAGE_QUERY_JOIN,
                  CHEETAH_CONTROL_MESSAGE_MAX_SIZE) == 0) {
        enum cheetah_join_table tbl = (receiver->payload_flow_id_arr[i] == 1)
                                          ? CHEETAH_JOIN_TABLE_PRIMARY
                                          : CHEETAH_JOIN_TABLE_SECONDARY;

        cheetah_join_update(join_state, receiver->payload_key_arr[i],
                            receiver->payload_row_id_arr[i], tbl);
        update_count++;
      }
    }
#endif /* CHEETAH_ENABLE_AGGREGATE_PROCESSING */

    if (started) {
      // cheetah_control_message_receive_acks(&worker_acks_left);
    }

    if (offloaded && !started && packets_received) {
      puts("HELLO");
      cheetah_debug_flush();
      cheetah_benchmark_packet_rate_start(&benchmarker, 0);
      system("python3 receive_ack_end.py &");
      started = 1;
    }

    if (!offloaded) {
      sleep(3);

      cheetah_debug("Offloading query...");
      cheetah_debug_flush();

      cheetah_control_message_offload_query(current_query_type);

      cheetah_debug("Success!\n");
      cheetah_debug_flush();

      offloaded = 1;
    }
  }

  cheetah_benchmark_packet_rate_stop(&benchmarker);

  cheetah_debug("Packets received: %" PRIu64 "\n", packets_received);
  cheetah_debug("Time elapsed (us): %f\n", benchmarker.time_elapsed_usec);

  cheetah_debug("Updates: %" PRIu64 "\n", update_count);

  cheetah_debug_flush();

  for (i = 0; i < groupby_state->current_entries; i++) {
    printf("Key: %d, Value: %d\n", groupby_state->ptr_array[i]->key,
           groupby_state->ptr_array[i]->value);
  }

  cheetah_packet_stream_receiver_free(receiver);
  cheetah_connection_state_free(state);

#endif /* CHEETAH_DISABLE_DPDK */

  return EXIT_SUCCESS;
}

int cheetah_control_flow_worker(int argc, char **const argv) {
  struct cheetah_connection_state *state;
  struct cheetah_fast_packet_stream *fast_pktstream;
  struct ether_addr dest_addr;
  struct cheetah_benchmarker benchmarker;
  struct cheetah_memory_sim *sim;

  register unsigned int i;
  register unsigned int j;

#ifdef CHEETAH_USE_MULTICORE_DPDK
  unsigned int lcore_id;
#endif /* CHEETAH_USE_MULTICORE_DPDK */

  const char *dataset_filename = CHEETAH_MEMORY_SIM_DATASET_PATH;

  cheetah_init(argc, argv);
  /* cheetah_packet_ether_addr_populate(&dest_addr, 0x24, 0x8a, 0x07, 0xc5,
   * 0x2e, 0xd0); */
  /* 3c:fd:fe:ab:de:d8 */
  cheetah_packet_ether_addr_populate(&dest_addr, CHEETAH_MASTER_MAC_1,
                                     CHEETAH_MASTER_MAC_2, CHEETAH_MASTER_MAC_3,
                                     CHEETAH_MASTER_MAC_4, CHEETAH_MASTER_MAC_5,
                                     CHEETAH_MASTER_MAC_6);

  state = cheetah_connection_state_create(0, dest_addr);

#ifdef CHEETAH_ENABLE_RESOURCE_LIMITS
  cheetah_debug("Limiting resources...");
  cheetah_debug_flush();

  cheetah_set_memory_limit(CHEETAH_WORKER_MEMORY_LIMIT);
  cheetah_set_cpu_core_limit(CHEETAH_WORKER_CORE_LIMIT);

  cheetah_debug("Success!\n");
  cheetah_debug_flush();
#endif /* CHEETAH_ENABLE_RESOURCE_LIMITS */

  fast_pktstream = cheetah_fast_packet_stream_create(state, BATCH_SIZE);

  sim = cheetah_memory_sim_create(dataset_filename, CHEETAH_MEMORY_ENTRY_COUNT);

  cheetah_debug("Waiting for query signal...");
  cheetah_debug_flush();

#ifndef CHEETAH_CONTROL_FLOW_WORKER_DEBUG
  cheetah_control_message_await_query(CHEETAH_WORKER_ID);
#endif /* CHEETAH_CONTROL_FLOW_WORKER_DEBUG */

  cheetah_debug("Success!\n");
  cheetah_debug_flush();

  cheetah_benchmark_packet_rate_start(&benchmarker, sim->entry_count);

  for (i = 0; i < (sim->entry_count / BATCH_SIZE); i++) {
    for (j = 0; j < BATCH_SIZE; j++) {

      cheetah_fast_packet_stream_modify(fast_pktstream, j,
#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
                                        sim->flow_id_arr[i * BATCH_SIZE + j],
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */
#ifdef CHEETAH_USE_KEY_AT_WORKER
                                        sim->key_arr[i * BATCH_SIZE + j],
#endif /* CHEETAH_USE_KEY_AT_WORKER */
                                        sim->value_arr[i * BATCH_SIZE + j],
                                        sim->row_id_arr[i * BATCH_SIZE + j]);

#ifdef CHEETAH_CONTROL_FLOW_WORKER_DEBUG
      /*
      rte_pktmbuf_dump(stdout, fast_pktstream->mbuf_burst_arr[j],
      CHEETAH_ETHERNET_FULL_LENGTH); fflush(stdout);
      */
#endif /* CHEETAH_CONTROL_FLOW_WORKER_DEBUG */
    }

#ifdef CHEETAH_USE_MULTICORE_DPDK
    RTE_LCORE_FOREACH(lcore_id) {
#endif /* CHEETAH_USE_MULTICORE_DPDK */
      cheetah_fast_packet_stream_send(fast_pktstream, state);

/*
      if (unlikely(i % 7) == 0) {
        usleep(7);
      }
*/
#ifdef CHEETAH_USE_MULTICORE_DPDK
    }
#endif /* CHEETAH_USE_MULTICORE_DPDK */

#ifdef CHEETAH_CONTROL_FLOW_WORKER_DEBUG
    /* break; */
#endif /* CHEETAH_CONTROL_FLOW_WORKER_DEBUG */
  }

  cheetah_benchmark_packet_rate_stop(&benchmarker);

  cheetah_control_message_send_ack();

  cheetah_debug("Packet rate: %f.\n", benchmarker.packet_rate);
  cheetah_debug("Time: %f.\n", benchmarker.time_elapsed_usec);
  cheetah_debug_flush();

  cheetah_fast_packet_stream_free(fast_pktstream);
  cheetah_memory_sim_free(sim);
  cheetah_connection_state_free(state);

  return EXIT_SUCCESS;
}
