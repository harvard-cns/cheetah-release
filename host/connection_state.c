/* Author: Muhammad Tirmazi
 * Harvard - Cloud, Networks & Systems (CNS) Lab
 */

#include "cheetah_dpdk.h"

static int cheetah_network_open(struct cheetah_connection_state *const state);

static int
cheetah_network_close(const struct cheetah_connection_state *const state);

struct cheetah_connection_state *
cheetah_connection_state_create(const uint16_t portid,
                                struct ether_addr dest_mac) {
  struct cheetah_connection_state *state;

  state = malloc(sizeof(struct cheetah_connection_state));
  state->portid = portid;

  /* Initialize all struct values to defaults */
  state->nb_rxd = CHEETAH_RXTX_DESC_DEFAULT;
  state->nb_txd = CHEETAH_RXTX_DESC_DEFAULT;

  state->tx_buffer = NULL;

  state->destination_mac = dest_mac;

  RTE_ASSERT(inet_pton(AF_INET, "192.168.1.2", &state->source_ip) != 0);
  RTE_ASSERT(inet_pton(AF_INET, "192.168.1.1", &state->destination_ip) != 0);

  /* Choosing arbitrary ports for now.
   * TODO: See if something better is required
   */
  state->udp_source_port = 4242;
  state->udp_destination_port = 4242;

  cheetah_network_open(state);

  return state;
}

void cheetah_connection_state_free(struct cheetah_connection_state *state) {
  cheetah_network_close(state);
  /* TODO: free tx_buffer if needed */
  free(state);
}

static int cheetah_network_open(struct cheetah_connection_state *const state) {
  int ret;
  /* using 1 lcore */
  /* struct lcore_queue_conf *qconf;*/

  /* designated initializer - used to configure device */
  struct rte_eth_conf default_port_config = {
      .rxmode =
          {
              .split_hdr_size = 0,
          },
      .txmode =
          {
              .mq_mode = ETH_MQ_TX_NONE,
          },
  };

  struct rte_eth_conf port_config = default_port_config;

  const char *shared_tx_packet_pool_name = "tx_packet_pool";
  const char *shared_rx_packet_pool_name = "rx_packet_pool";

  state->proc_type = rte_eal_process_type();

  if (state->proc_type == RTE_PROC_SECONDARY) {
    cheetah_debug("Worker is secondary process.\n");
    cheetah_debug("Looking up primary worker's "
                  "RX and TX memory pools...");
    cheetah_debug_flush();

    state->tx_packet_pool = rte_mempool_lookup(shared_tx_packet_pool_name);
    if (state->tx_packet_pool == NULL)
      rte_exit(EXIT_FAILURE, "Cannot lookup tx packet pool\n");

    state->rx_packet_pool = rte_mempool_lookup(shared_rx_packet_pool_name);
    if (state->rx_packet_pool == NULL)
      rte_exit(EXIT_FAILURE, "Cannot lookup rx packet pool\n");

    cheetah_debug("Success!\n");
    cheetah_debug_flush();

    return 0;

  } else {
    cheetah_debug("Worker is primary process.\n");
    cheetah_debug("Allocating RX and TX memory pools...");
    cheetah_debug_flush();

    state->tx_packet_pool = rte_pktmbuf_pool_create(
        shared_tx_packet_pool_name, CHEETAH_PACKET_MBUF, 128, 0,
        CHEETAH_MBUF_DATA_SIZE, rte_socket_id());

    if (state->tx_packet_pool == NULL)
      rte_exit(EXIT_FAILURE, "Cannot init tx packet pool\n");

    state->rx_packet_pool = rte_pktmbuf_pool_create(
        shared_rx_packet_pool_name, CHEETAH_PACKET_MBUF,
        CHEETAH_MEMPOOL_CACHE_SIZE, 0, CHEETAH_MBUF_DATA_SIZE, rte_socket_id());

    if (state->rx_packet_pool == NULL)
      rte_exit(EXIT_FAILURE, "Cannot init rx packet pool\n");
  }

  cheetah_debug("Success!\n");
  cheetah_debug_flush();

  cheetah_debug("Configuring device...");
  cheetah_debug_flush();

  rte_eth_dev_info_get(state->portid, &state->dev_info);

  if (state->dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
    port_config.txmode.offloads |= DEV_TX_OFFLOAD_MBUF_FAST_FREE;

  ret = rte_eth_dev_configure(state->portid, 1, 1, &port_config);
  if (ret < 0)
    rte_exit(EXIT_FAILURE, "Cannot configure device: err=%d, port=%u\n", ret,
             state->portid);

  cheetah_debug("Success!\n");
  cheetah_debug("Adjusting descriptor numbers...");
  cheetah_debug_flush();

  ret = rte_eth_dev_adjust_nb_rx_tx_desc(state->portid, &state->nb_rxd,
                                         &state->nb_txd);

  if (ret < 0)
    rte_exit(EXIT_FAILURE,
             "Cannot adjust number of descriptors: err=%d, port=%u\n", ret,
             state->portid);

  cheetah_debug("Success!\n");

  /* Get mac address of device */
  rte_eth_macaddr_get(state->portid, &state->mac_address);

  /* For some reason, the DPDK docs flush stdout
   * before initializing RX / TX queues */
  fflush(stdout); /* Why am I doing this? Check later */

  cheetah_debug("Initializing RX queue...");
  cheetah_debug_flush();

  state->rxq_conf = state->dev_info.default_rxconf;
  state->rxq_conf.offloads = port_config.rxmode.offloads;

  ret =
      rte_eth_rx_queue_setup(state->portid, CHEETAH_RX_QUEUE_ID, state->nb_rxd,
                             rte_eth_dev_socket_id(state->portid),
                             &state->rxq_conf, state->rx_packet_pool);
  if (ret < 0)
    rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup: err=%d, port=%u\n", ret,
             state->portid);

  cheetah_debug("Success!\n");
  cheetah_debug_flush();

  fflush(stdout); /* Why am I doing this? (2) Check later */

  cheetah_debug("Initializing TX queue...");
  cheetah_debug_flush();

  state->txq_conf = state->dev_info.default_txconf;
  state->txq_conf.offloads = port_config.txmode.offloads;

  ret = rte_eth_tx_queue_setup(
      state->portid, CHEETAH_TX_QUEUE_ID, state->nb_txd,
      rte_eth_dev_socket_id(state->portid), &state->txq_conf);

  if (ret < 0)
    rte_exit(EXIT_FAILURE, "rte_eth_tx_queue_setup:err=%d, port=%u\n", ret,
             state->portid);

  cheetah_debug("Success!\n");
  cheetah_debug("Initializing TX buffers...");
  cheetah_debug_flush();

  state->tx_buffer = rte_zmalloc_socket(
      "tx_buffer", RTE_ETH_TX_BUFFER_SIZE(CHEETAH_MAX_PKT_BURST_DEFAULT), 0,
      rte_eth_dev_socket_id(state->portid));

  if (state->tx_buffer == NULL)
    rte_exit(EXIT_FAILURE, "Cannot allocate buffer for tx on port %u\n",
             state->portid);

  rte_eth_tx_buffer_init(state->tx_buffer, CHEETAH_MAX_PKT_BURST_DEFAULT);

  cheetah_debug("Success!\n");
  cheetah_debug_flush();

  /* TODO: Set tx buffer error callback if needed later */

  cheetah_debug("Starting device...");
  cheetah_debug_flush();

  ret = rte_eth_dev_start(state->portid);
  if (ret < 0)
    rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%u\n", ret,
             state->portid);

  cheetah_debug("Success!\n");
  cheetah_debug_flush();

  return 0;
}

static int
cheetah_network_close(const struct cheetah_connection_state *const state) {
  cheetah_debug("Closing device on port %d....", state->portid);
  cheetah_debug_flush();

  rte_eth_dev_stop(state->portid);
  rte_eth_dev_close(state->portid);

  cheetah_debug("Success!\n");
  cheetah_debug_flush();

  return 0;
}
