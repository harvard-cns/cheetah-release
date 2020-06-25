/* #define CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS */
#ifdef CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS
#define _GNU_SOURCE
#include "cheetah_aggregate_processing.h"
#include <assert.h>
#include <search.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include "cheetah_dpdk.h"
#endif

struct cheetah_aggregate_processing_topn_state *
cheetah_aggregate_processing_topn_init(const int n) {

  struct cheetah_aggregate_processing_topn_state *state =
      malloc(sizeof(struct cheetah_aggregate_processing_topn_state));

  state->n = n;
  state->current_entries = 0;
  state->result_array = malloc(sizeof(uint32_t) * n);
  return state;
}

void cheetah_aggregate_processing_topn_free(
    struct cheetah_aggregate_processing_topn_state *state) {
  free(state->result_array);
  free(state);
}

void cheetah_aggregate_processing_topn_update(
    struct cheetah_aggregate_processing_topn_state *state,
    const uint32_t value) {

  register int i;
  register int j;

  if (state->current_entries < state->n) {
    i = state->current_entries - 1;

    while (i >= 0 && state->result_array[i] < value) {
      i--;
    }

    if (i < ((int)state->current_entries - 1)) {
      for (j = state->current_entries - 1; j > i; j--) {
        state->result_array[j + 1] = state->result_array[j];
      }
    }

    state->result_array[i + 1] = value;
    state->current_entries++;
  }

  else {
    i = state->n - 1;
    while (i >= 0 && state->result_array[i] < value) {
      i--;
    }

    if (i < ((int)state->n - 1)) {
      for (j = state->n - 2; j > i; j--) {
        state->result_array[j + 1] = state->result_array[j];
      }

      state->result_array[i + 1] = value;
    }
  }
}

struct cheetah_aggregate_processing_distinct_state *
cheetah_aggregate_processing_distinct_init(const uint32_t size) {

  struct cheetah_aggregate_processing_distinct_state *state;

  state = calloc(1, sizeof(struct cheetah_aggregate_processing_distinct_state));

  state->size = size;
  state->current_entries = 0;
  state->current_hash_bins = 0;

  hcreate_r(size, &state->htab);

  state->result_array = malloc(sizeof(uint64_t) * size);
  state->ptr_array = malloc(
      sizeof(struct cheetah_aggregate_processing_distinct_entries *) * size);

  return state;
}

static void
free_distinct_entry(struct cheetah_aggregate_processing_distinct_entries *e) {
  if (e) {
    free(e->next);
    free(e);
  }
}

void cheetah_aggregate_processing_distinct_free(
    struct cheetah_aggregate_processing_distinct_state *state) {

  register uint32_t i;

  hdestroy_r(&state->htab);

  for (i = 0; i < state->current_entries; i++) {
    free_distinct_entry(state->ptr_array[i]);
  }

  free(state->result_array);
  free(state);
}

void cheetah_aggregate_processing_distinct_update(
    struct cheetah_aggregate_processing_distinct_state *state,
    const uint64_t value) {

  /* 8 chars = 8 * 8 = 64 bits */
  char key[8];
  ENTRY e, *ep;
  unsigned int n;
  struct cheetah_aggregate_processing_distinct_entries *bin;
  struct cheetah_aggregate_processing_distinct_entries *bin_prev;

  n = 0;
  bin = NULL;
  bin_prev = NULL;

  memcpy(key, &value, 8);

  e.key = key;
  n = hsearch_r(e, FIND, &ep, &state->htab);

  if (n) {

    bin = (struct cheetah_aggregate_processing_distinct_entries *)ep->data;

#ifdef CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS
    assert(bin);
#else
    RTE_ASSERT(bin);
#endif

    while (bin && bin->value != value) {
      bin_prev = bin;
      bin = bin->next;
    }

    if (!bin) {
#ifdef CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS
      assert(bin_prev);
      assert(bin_prev->next == NULL);
#else
      RTE_ASSERT(bin_prev);
      RTE_ASSERT(bin_prev->next == NULL);
#endif /* CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS */

      bin_prev->next =
          malloc(sizeof(struct cheetah_aggregate_processing_distinct_entries));

      bin = bin_prev->next;
      bin->value = value;
      bin->next = NULL;

      state->result_array[state->current_entries] = value;
      state->current_entries++;
    }
  }

  else {
    /* add it to hash table */
    state->result_array[state->current_entries] = value;
    state->current_entries++;

    bin = malloc(sizeof(struct cheetah_aggregate_processing_distinct_entries));
    bin->next = NULL;
    bin->value = value;

    e.data = (void *)bin;
    n = hsearch_r(e, ENTER, &ep, &state->htab);

#ifdef CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS
    assert(n);
#else
    RTE_ASSERT(n);
#endif /* CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS */

    state->ptr_array[state->current_hash_bins] = bin;
    state->current_hash_bins++;
  }
}

struct cheetah_aggregate_processing_groupby_state *
cheetah_aggregate_processing_groupby_init(const uint32_t size) {

  struct cheetah_aggregate_processing_groupby_state *state;

  state = calloc(1, sizeof(struct cheetah_aggregate_processing_groupby_state));

  state->size = size;
  state->current_entries = 0;
  state->current_hash_bins = 0;

  hcreate_r(size, &state->htab);

  state->ptr_array =
      malloc(sizeof(struct cheetah_aggregate_processing_group_state *) * size);

  return state;
}

static void
free_groupby_entry(struct cheetah_aggregate_processing_groupby_entries *e) {
  if (e) {
    free(e->next);
    free(e);
  }
}

void cheetah_aggregate_processing_groupby_free(
    struct cheetah_aggregate_processing_groupby_state *state) {
  register uint32_t i;

  hdestroy_r(&state->htab);

  for (i = 0; i < state->current_entries; i++) {
    free_groupby_entry(state->ptr_array[i]);
  }

  free(state);
}

void cheetah_aggregate_processing_groupby_update(
    struct cheetah_aggregate_processing_groupby_state *state,
    const uint32_t key, const uint32_t value) {
  /* 4 chars = 4 * 8 = 32 bits */
  char formatted_key[4];
  ENTRY e;
  ENTRY *ep;

  unsigned int n;
  struct cheetah_aggregate_processing_groupby_entries *bin;
  struct cheetah_aggregate_processing_groupby_entries *bin_prev;

  n = 0;
  bin = NULL;
  bin_prev = NULL;

  memcpy(formatted_key, &key, 4);

  e.key = formatted_key;
  n = hsearch_r(e, FIND, &ep, &state->htab);

  /* found */
  if (n) {
    bin = (struct cheetah_aggregate_processing_groupby_entries *)ep->data;

#ifdef CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS
    assert(bin);
#else
    RTE_ASSERT(bin);
#endif /* CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS */

    while (bin && bin->key != key) {
      bin_prev = bin;
      bin = bin->next;
    }

    if (!bin) {
#ifdef CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS
      assert(bin_prev);
      assert(bin_prev->next == NULL);
#else
      RTE_ASSERT(bin_prev);
      RTE_ASSERT(bin_prev->next == NULL);
#endif /* CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS */

      bin_prev->next =
          malloc(sizeof(struct cheetah_aggregate_processing_groupby_entries));

      bin = bin_prev->next;
      bin->value = value;
      bin->key = key;
      bin->next = NULL;

      state->ptr_array[state->current_entries] = bin;
      state->current_entries++;
    } else {
#ifdef CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS
      assert(bin->key == key);
#else
      RTE_ASSERT(bin->key == key);
#endif /* CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS */
      if (bin->value < value) {
        bin->value = value;
      }
    }
  } else {
    bin = malloc(sizeof(struct cheetah_aggregate_processing_groupby_entries));
    bin->next = NULL;
    bin->key = key;
    bin->value = value;

    e.data = (void *)bin;
    n = hsearch_r(e, ENTER, &ep, &state->htab);

#ifdef CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS
    assert(n);
#else
    RTE_ASSERT(n);
#endif /* CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS */

    state->ptr_array[state->current_entries] = bin;
    state->current_entries++;
    state->current_hash_bins++;
  }
}

#ifdef CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS
int main() {
  struct cheetah_aggregate_processing_topn_state *state;
  struct cheetah_aggregate_processing_distinct_state *distinct_state;
  struct cheetah_aggregate_processing_groupby_state *groupby_state;
  register int i;

  puts("Testing groupby");
  groupby_state = cheetah_aggregate_processing_groupby_init(4);

  cheetah_aggregate_processing_groupby_update(groupby_state, 1, 3);

  assert(groupby_state->current_entries == 1);
  assert(groupby_state->current_hash_bins == 1);
  assert(groupby_state->ptr_array[0]->key == 1);
  assert(groupby_state->ptr_array[0]->value == 3);

  cheetah_aggregate_processing_groupby_update(groupby_state, 42, 11);
  cheetah_aggregate_processing_groupby_update(groupby_state, 1, 5);

  assert(groupby_state->current_entries == 2);
  assert(groupby_state->ptr_array[0]->key == 1);
  assert(groupby_state->ptr_array[0]->value == 5);
  assert(groupby_state->ptr_array[1]->key == 42);
  assert(groupby_state->ptr_array[1]->value == 11);

  cheetah_aggregate_processing_groupby_update(groupby_state, 42, 10);

  assert(groupby_state->current_entries == 2);
  assert(groupby_state->ptr_array[0]->key == 1);
  assert(groupby_state->ptr_array[0]->value == 5);
  assert(groupby_state->ptr_array[1]->key == 42);
  assert(groupby_state->ptr_array[1]->value == 11);

  cheetah_aggregate_processing_groupby_free(groupby_state);
  puts("All tests passed.");

  puts("Testing Top-N");
  state = cheetah_aggregate_processing_topn_init(4);

  cheetah_aggregate_processing_topn_update(state, 11);
  cheetah_aggregate_processing_topn_update(state, 9);
  cheetah_aggregate_processing_topn_update(state, 13);
  cheetah_aggregate_processing_topn_update(state, 10);

  assert(state->result_array[0] == 13);
  assert(state->result_array[1] == 11);
  assert(state->result_array[2] == 10);
  assert(state->result_array[3] == 9);

  cheetah_aggregate_processing_topn_update(state, 8);

  assert(state->result_array[0] == 13);
  assert(state->result_array[1] == 11);
  assert(state->result_array[2] == 10);
  assert(state->result_array[3] == 9);

  cheetah_aggregate_processing_topn_update(state, 15);

  assert(state->result_array[0] == 15);
  assert(state->result_array[1] == 13);
  assert(state->result_array[2] == 11);
  assert(state->result_array[3] == 10);

  cheetah_aggregate_processing_topn_update(state, 11);

  assert(state->result_array[0] == 15);
  assert(state->result_array[1] == 13);
  assert(state->result_array[2] == 11);
  assert(state->result_array[3] == 11);

  cheetah_aggregate_processing_topn_update(state, 12);

  assert(state->result_array[0] == 15);
  assert(state->result_array[1] == 13);
  assert(state->result_array[2] == 12);
  assert(state->result_array[3] == 11);

  cheetah_aggregate_processing_topn_free(state);

  puts("All tests passed.");

  puts("Testing Distinct");

  distinct_state = cheetah_aggregate_processing_distinct_init(10);

  cheetah_aggregate_processing_distinct_update(distinct_state, 10);

  cheetah_aggregate_processing_distinct_update(distinct_state, 10);

  assert(distinct_state->current_entries == 1);
  assert(distinct_state->current_hash_bins == 1);
  assert(distinct_state->result_array[0] == 10);

  cheetah_aggregate_processing_distinct_update(distinct_state, 11);

  assert(distinct_state->current_entries == 2);
  assert(distinct_state->result_array[0] == 10);
  assert(distinct_state->result_array[1] == 11);
  assert(distinct_state->current_hash_bins <= 2);

  cheetah_aggregate_processing_distinct_update(distinct_state, 10);

  assert(distinct_state->current_entries == 2);
  assert(distinct_state->result_array[0] == 10);
  assert(distinct_state->result_array[1] == 11);
  assert(distinct_state->current_hash_bins <= 2);

  for (i = 0; i < 100000; i++) {
    cheetah_aggregate_processing_distinct_update(distinct_state, 11);
  }

  assert(distinct_state->current_entries == 2);
  assert(distinct_state->result_array[0] == 10);
  assert(distinct_state->result_array[1] == 11);
  assert(distinct_state->current_hash_bins <= 2);

  cheetah_aggregate_processing_distinct_free(distinct_state);
  puts("All tests passed");
}

#endif /* CHEETAH_AGGREGATE_PROCESSING_ENABLE_TESTS */
