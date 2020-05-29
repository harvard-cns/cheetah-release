#ifndef CHEETAH_BENCHMARK_H
#define CHEETAH_BENCHMARK_H

struct cheetah_benchmarker {
  struct timeval tv_start;
  struct timeval tv_end;
  struct timeval tv_difference;
  unsigned int packets_to_send;
  double time_elapsed_usec;
  double packet_rate;
};

void cheetah_benchmark_packet_rate_start(
    struct cheetah_benchmarker *const benchmarker,
    const unsigned int packets_to_send);

void cheetah_benchmark_packet_rate_stop(
    struct cheetah_benchmarker *const benchmarker);

#endif /* CHEETAH_BENCHMARK_H */
