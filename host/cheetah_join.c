#include "cheetah_join.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct cheetah_join_state *cheetah_join_init(const uint64_t size,
                                             const uint64_t hash_size) {
  assert(size > 0);

  int rtn;
  struct cheetah_join_state *state;

  state = malloc(sizeof(struct cheetah_join_state));
  assert(state != NULL);

  state->entries = malloc(sizeof(struct cheetah_join_entry) * size);
  assert(state->entries != NULL);

  state->size = size;
  state->current_entries = 0;

  rtn = hcreate_r(hash_size, &state->htab);
  assert(rtn != 0);

  return state;
}

struct hash_bin {
  uint32_t key;
  uint32_t *row_ids;
  uint64_t row_id_count;
  uint64_t row_id_cap;

  struct hash_bin *next;
};

void cheetah_join_free(struct cheetah_join_state *state) {
  assert(state != NULL);
  assert(state->entries != NULL);

  hdestroy_r(&state->htab);

  free(state->entries);
  free(state);
}

void cheetah_join_update(struct cheetah_join_state *state, const uint32_t key,
                         const uint32_t row_id,
                         const enum cheetah_join_table tbl) {
  /* 4 chars = 4 * 8 = 32 bits */
  char formatted_key[4];
  ENTRY e;
  ENTRY *ep;
  struct hash_bin *bin;
  struct hash_bin *bin_prev;

  unsigned int n;

  n = 0;

  /* 4 because char is 1 byte */
  memcpy(formatted_key, &key, 4);

  e.key = formatted_key;
  n = hsearch_r(e, FIND, &ep, &state->htab);

  if (n) {
    bin = (struct hash_bin *)ep->data;

    assert(bin);

    while (bin && bin->key != key) {
      bin_prev = bin;
      bin = bin->next;
    }

    if (!bin) {
      if (tbl == CHEETAH_JOIN_TABLE_PRIMARY) {
        assert(bin_prev);
        assert(bin_prev->next == NULL);

        bin_prev->next = malloc(sizeof(struct hash_bin));

        bin = bin_prev->next;
        bin->key = key;
        bin->next = NULL;
        bin->row_id_cap = 1024;
        bin->row_ids = malloc(sizeof(uint32_t) * bin->row_id_cap);
        assert(bin->row_ids != NULL);
        bin->row_id_count = 1;
        bin->row_ids[0] = row_id;
      }
    } else {
      assert(bin->key == key);

      if (tbl == CHEETAH_JOIN_TABLE_PRIMARY) {
        if (bin->row_id_cap == bin->row_id_count) {
          uint64_t next_size = 2 * bin->row_id_cap;
          uint32_t *temp = malloc(sizeof(uint32_t) * next_size);
          memcpy(temp, bin->row_ids, sizeof(uint32_t) * bin->row_id_cap);
          free(bin->row_ids);
          bin->row_ids = temp;
          bin->row_id_cap = next_size;
        }

        bin->row_ids[bin->row_id_count] = row_id;
        bin->row_id_count++;
      } else {
        uint64_t i;

        for (i = 0; i < bin->row_id_count; i++) {
          assert(state->current_entries < state->size);
          struct cheetah_join_entry *curr_entry =
              &state->entries[state->current_entries];

          curr_entry->tbl_1_row = bin->row_ids[i];
          curr_entry->tbl_2_row = row_id;

          state->current_entries++;
        }
      }
    }
  } else {
    bin = malloc(sizeof(struct hash_bin));
    assert(bin != NULL);
    bin->key = key;
    bin->next = NULL;
    bin->row_id_cap = 1024;
    bin->row_ids = malloc(sizeof(uint32_t) * bin->row_id_cap);
    assert(bin->row_ids != NULL);
    bin->row_id_count = 1;
    bin->row_ids[0] = row_id;

    e.data = (void *)bin;

    n = hsearch_r(e, ENTER, &ep, &state->htab);
    assert(n);
  }
}

#ifdef CHEETAH_JOIN_ENABLE_TESTS
int main() {
  struct cheetah_join_state *state;

  puts("Trivial test.");
  state = cheetah_join_init(40000, 15000);
  assert(state != NULL);
  cheetah_join_free(state);
  puts("Passed.");

  puts("Disjoint join test.");
  state = cheetah_join_init(40000, 15000);

  cheetah_join_update(state, 13, 2, CHEETAH_JOIN_TABLE_PRIMARY);
  cheetah_join_update(state, 14, 1, CHEETAH_JOIN_TABLE_SECONDARY);

  assert(state->current_entries == 0);
  cheetah_join_free(state);
  puts("Passed.");

  puts("Unique key test.");
  state = cheetah_join_init(40000, 15000);

  cheetah_join_update(state, 14, 2, CHEETAH_JOIN_TABLE_PRIMARY);
  cheetah_join_update(state, 14, 1, CHEETAH_JOIN_TABLE_SECONDARY);

  assert(state->current_entries == 1);
  cheetah_join_free(state);
  puts("Passed.");

  puts("Duplicate key test.");
  state = cheetah_join_init(40000, 15000);

  cheetah_join_update(state, 14, 2, CHEETAH_JOIN_TABLE_PRIMARY);
  cheetah_join_update(state, 14, 3, CHEETAH_JOIN_TABLE_PRIMARY);
  cheetah_join_update(state, 14, 1, CHEETAH_JOIN_TABLE_SECONDARY);

  assert(state->current_entries == 2);
  cheetah_join_free(state);
  puts("Passed.");
}

#endif /* CHEETAH_JOIN_ENABLE_TESTS */
