#include "cheetah_dpdk.h"

int main(int argc, char **const argv) {
  /* cheetah_set_memory_limit(CHEETAH_WORKER_MEMORY_LIMIT); */
  return cheetah_control_flow_master(argc, argv);
}
