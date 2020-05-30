#include "cheetah_dpdk.h"

void cheetah_benchmark_packet_rate_start(
    struct cheetah_benchmarker *const benchmarker,
    const unsigned int packets_to_send) {
  benchmarker->packets_to_send = packets_to_send;

  gettimeofday(&benchmarker->tv_start, NULL);
}

void cheetah_benchmark_packet_rate_stop(
    struct cheetah_benchmarker *const benchmarker) {
  gettimeofday(&benchmarker->tv_end, NULL);
  timersub(&benchmarker->tv_end, &benchmarker->tv_start,
           &benchmarker->tv_difference);

  benchmarker->time_elapsed_usec =
      (1000000.0 * benchmarker->tv_difference.tv_sec) +
      benchmarker->tv_difference.tv_usec;
  benchmarker->packet_rate = (benchmarker->packets_to_send * 1000000.0) /
                             benchmarker->time_elapsed_usec;
}
