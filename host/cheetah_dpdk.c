/* Author: Muhammad Tirmazi
 * Harvard - Cloud, Networks & Systems (CNS) Lab
 */

#include "cheetah_dpdk.h"

static void cheetah_signal_handler(const int signum) {
  if (signum == SIGINT || signum == SIGTERM) {
    printf("\n\nSignal %d received, preparing to exit...\n", signum);
    cheetah_global_force_quit = true;
  }
}

int cheetah_init(const int argc, char **argv) {
  /* Initialize DPDK's EAL */
  if (rte_eal_init(argc, argv) < 0)
    rte_exit(EXIT_FAILURE, "Invalid EAL arguments");

  cheetah_global_force_quit = false;
  signal(SIGINT, cheetah_signal_handler);
  signal(SIGTERM, cheetah_signal_handler);

  return 0;
}
