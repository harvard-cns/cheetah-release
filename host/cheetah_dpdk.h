/* Author: Muhammad Tirmazi
 * Harvard - Cloud, Networks & Systems (CNS) Lab
 */

#ifndef CHEETAH_DPDK_H
#define CHEETAH_DPDK_H

/* TODO: convert this to the
   "each header only including what it needs"
   model advocated by the Google C++ convention
   (even though this is C, not C++)
*/

#include <arpa/inet.h> /* <- Testing this */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
/* #include <netinet/ether.h> */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

/* Used in cheetah_aggregate_processing */
#include <search.h>

/* Used in cheetah_resource_manager */
#include <sys/resource.h>

#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_debug.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_interrupts.h>
#include <rte_ip.h>
#include <rte_launch.h>
#include <rte_lcore.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_mbuf.h>
#include <rte_memcpy.h>
#include <rte_memory.h>
#include <rte_mempool.h>
#include <rte_per_lcore.h>
#include <rte_prefetch.h>
#include <rte_random.h>
#include <rte_udp.h>

#include "cheetah_constants.h"

#define CHEETAH_LOG_DEBUG
/* #define CHEETAH_ONLINE_DEBUG */

#define CHEETAH_DEBUG_COLOR_GREEN "\x1b[32m"
#define CHEETAH_DEBUG_COLOR_RESET "\x1b[0m"
#define CHEETAH_DEBUG_COLOR_CYAN "\x1b[36m"

#ifdef CHEETAH_MEMORY_DEBUG
#define cheetah_memory_debug(...)                                              \
  {                                                                            \
    fprintf(stderr, "\x1b[32m");                                               \
    fprintf(stderr, __VA_ARGS__);                                              \
    fprintf(stderr, "\x1b[0m");                                                \
  }
#define cheetah_memory_debug_flush() fflush(stderr)
#else
#define cheetah_memory_debug(...)
#define cheetah_memory_debug_flush()
#endif

#ifdef CHEETAH_LOG_DEBUG
#define cheetah_debug(...)                                                     \
  {                                                                            \
    fprintf(stderr, "\x1b[36m");                                               \
    fprintf(stderr, __VA_ARGS__);                                              \
    fprintf(stderr, "\x1b[0m");                                                \
  }

#define cheetah_debug_flush() fflush(stderr)
#else
#define cheetah_debug(...)
#define cheetah_debug_flush()
#endif

#ifdef CHEETAH_ONLINE_DEBUG
#define cheetah_online_debug(...) fprintf(stderr, __VA_ARGS__)
#define cheetah_online_debug_flush() fflush(stderr)
#else
#define cheetah_online_debug(...)
#define cheetah_online_debug_flush()
#endif

/* For variables unused *for good reason* e.g
 * used inside assert / RTE_ASSERT */
#define cheetah_unused_var(x) ((void)(x))

int cheetah_init(const int argc, char **argv);

#include "connection_state.h"
#include "packet_manager.h"
#include "packet_stream_receiver.h"

#include "fast_packet_stream.h"
#include "packet_crafter.h"

#ifdef CHEETAH_USE_HDFS_BACKEND
#include "cheetah_hdfs_backend.h"
#else
#include "cheetah_memory_sim.h"
#endif /* CHEETAH_USE_HDFS_BACKEND */

#include "cheetah_aggregate_processing.h"
#include "cheetah_benchmark.h"
#include "cheetah_control_flow.h"
#include "cheetah_control_message.h"
#include "cheetah_join.h"
#include "cheetah_resource_manager.h"
#include "control_message_manager.h"

volatile bool cheetah_global_force_quit;

#endif /* CHEETAH_DPDK_H */
