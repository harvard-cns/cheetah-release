#ifndef CHEETAH_AGGREGATE_PROCESSING_H
#define CHEETAH_AGGREGATE_PROCESSING_H

struct cheetah_aggregate_processing_topn_state {
  uint32_t n;
  uint32_t *result_array;
  uint32_t current_entries;
};

struct cheetah_aggregate_processing_topn_state *
cheetah_aggregate_processing_topn_init(const int n);

void cheetah_aggregate_processing_topn_free(
    struct cheetah_aggregate_processing_topn_state *state);

void cheetah_aggregate_processing_topn_update(
    struct cheetah_aggregate_processing_topn_state *state,
    const uint32_t value);

struct cheetah_aggregate_processing_distinct_entries {
  uint64_t value;
  struct cheetah_aggregate_processing_distinct_entries *next;
};

struct cheetah_aggregate_processing_distinct_state {
  uint32_t current_entries;
  uint32_t current_hash_bins;
  uint32_t size;
  struct hsearch_data htab;
  uint64_t *result_array;
  struct cheetah_aggregate_processing_distinct_entries **ptr_array;
};

struct cheetah_aggregate_processing_distinct_state *
cheetah_aggregate_processing_distinct_init(const uint32_t size);

void cheetah_aggregate_processing_distinct_free(
    struct cheetah_aggregate_processing_distinct_state *state);

void cheetah_aggregate_processing_distinct_update(
    struct cheetah_aggregate_processing_distinct_state *state,
    const uint64_t value);

struct cheetah_aggregate_processing_groupby_entries {
  uint32_t key;
  uint32_t value;
  struct cheetah_aggregate_processing_groupby_entries *next;
};

struct cheetah_aggregate_processing_groupby_state {
  uint32_t current_entries;
  uint32_t current_hash_bins;
  uint32_t size;
  struct hsearch_data htab;
  struct cheetah_aggregate_processing_groupby_entries **ptr_array;
};

struct cheetah_aggregate_processing_groupby_state *
cheetah_aggregate_processing_groupby_init(const uint32_t size);

void cheetah_aggregate_processing_groupby_free(
    struct cheetah_aggregate_processing_groupby_state *state);

void cheetah_aggregate_processing_groupby_update(
    struct cheetah_aggregate_processing_groupby_state *state,
    const uint32_t key, const uint32_t value);

#endif /* CHEETAH_AGGREGATE_PROCESSING_H */
