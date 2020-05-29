#include "cheetah_dpdk.h"

void cheetah_set_memory_limit(const cheetah_resource_limit_t lim) {
  rlim_t internal_limit;
  struct rlimit memory_limit;

  internal_limit = (rlim_t)lim;
  cheetah_debug("Setting memory limit to %ju...", (uintmax_t)internal_limit);
  cheetah_debug_flush();

  memory_limit.rlim_cur = internal_limit;
  memory_limit.rlim_max = internal_limit;
  setrlimit(RLIMIT_AS, &memory_limit);

  cheetah_debug("Done.\n");
  cheetah_debug_flush();
}

void cheetah_set_cpu_core_limit(const cheetah_resource_limit_t lim) {
  rlim_t internal_limit;
  struct rlimit nproc_limit;

  internal_limit = (rlim_t)lim;

  cheetah_debug("Setting core limit to %ju...", (uintmax_t)internal_limit);
  cheetah_debug("(using RLIMIT_NPROC)...");
  cheetah_debug_flush();

  nproc_limit.rlim_cur = internal_limit;
  nproc_limit.rlim_max = internal_limit;
  setrlimit(RLIMIT_NPROC, &nproc_limit);

  cheetah_debug("Done.\n");
  cheetah_debug_flush();
}
