#ifndef CHEETAH_JOIN_H
#define CHEETAH_JOIN_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <search.h>
#include <stdint.h>
#include <stdlib.h>

enum cheetah_join_table {
  CHEETAH_JOIN_TABLE_PRIMARY,
  CHEETAH_JOIN_TABLE_SECONDARY
};

/* Implements aggregate join at master */

struct cheetah_join_entry {
  uint32_t tbl_1_row;
  uint32_t tbl_2_row;
};

struct cheetah_join_state {
  struct cheetah_join_entry *entries;
  uint64_t size;
  uint64_t current_entries;

  struct hsearch_data htab;
};

struct cheetah_join_state *cheetah_join_init(const uint64_t size,
                                             const uint64_t hash_size);

void cheetah_join_free(struct cheetah_join_state *state);

void cheetah_join_update(struct cheetah_join_state *state, const uint32_t key,
                         const uint32_t row_id,
                         const enum cheetah_join_table tbl);
#endif /* CHEETAH_JOIN_H */
