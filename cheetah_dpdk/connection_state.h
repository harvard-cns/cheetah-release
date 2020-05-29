/* Author: Muhammad Tirmazi
 * Harvard - Cloud, Networks & Systems Group (CNS)
 */

#ifndef CHEETAH_CONNECTION_STATE
#define CHEETAH_CONNECTION_STATE

/* DPDK state for an ethernet device */
struct cheetah_connection_state {
  /* The port for the ethernet device
   * make sure the device is bound to DPDK */
  uint16_t portid;

  /* The receiver queue configuration */
  struct rte_eth_rxconf rxq_conf;

  /* The transmit queue configuration */
  struct rte_eth_txconf txq_conf;

  /* The device information */
  struct rte_eth_dev_info dev_info;

  /* rx and tx descriptors */
  uint16_t nb_rxd, nb_txd;

  /* Mac address of device */
  struct ether_addr mac_address;

  /* Memory pool for packet buffering */
  struct rte_mempool *pktmbuf_pool;

  /* TX buffer for the device */
  struct rte_eth_dev_tx_buffer *tx_buffer;

  /* Memory pool for the packets sent */
  struct rte_mempool *tx_packet_pool;

  /* Memory pool for the packets received */
  struct rte_mempool *rx_packet_pool;

  /* MAC Address of the destination */
  struct ether_addr destination_mac;

  /* IP Address of device */
  uint32_t source_ip;

  /* IP Address of destination */
  uint32_t destination_ip;

  /* UDP source port */
  uint16_t udp_source_port;

  /* UDP destination port */
  uint16_t udp_destination_port;

  /* process type */
  enum rte_proc_type_t proc_type;
};

/* state constructor */
struct cheetah_connection_state *
cheetah_connection_state_create(const uint16_t portid,
                                struct ether_addr dest_mac);

/* state destructor */
void cheetah_connection_state_free(struct cheetah_connection_state *state);

/* Opens a DPDK port linked to an ethernet device */
/* int cheetah_network_open(struct cheetah_connection_state* const state); */

/* Closes a DPDK port linked to an ethernet device */
/* int cheetah_network_close(const struct cheetah_connection_state* const
 * state); */

#endif /* CHEETAH_CONNECTION_STATE */
