#include "cheetah_dpdk.h"

struct cheetah_memory_sim *cheetah_memory_sim_create(const char *filename,
                                                     const size_t size) {
  struct cheetah_memory_sim *sim;
  FILE *stream;
  int scan_output;

#ifdef CHEETAH_USE_STRING_VALUES
  char value[CHEETAH_CSV_ENTRY_MAX_LENGTH];
#else
  int value;
#endif /* CHEETAH_USE_STRING_VALUES */

#ifdef CHEETAH_USE_KEY_AT_WORKER
  int key;
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  int flow_id;
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

  cheetah_unused_var(key);
  cheetah_unused_var(value);
  cheetah_unused_var(flow_id);

  register unsigned int i;

  sim = malloc(sizeof(struct cheetah_memory_sim));
  RTE_ASSERT(sim != NULL);

  sim->array_count = size;

  cheetah_debug("Allocating memory for dataset...");
  cheetah_debug_flush();

  sim->row_id_arr = malloc(sizeof(cheetah_header_field_t) * size);
  RTE_ASSERT(sim->row_id_arr != NULL);

#ifdef CHEETAH_USE_STRING_VALUES
  sim->value_arr = malloc(sizeof(char *) * size);
#elif defined(CHEETAH_USE_64_BIT_VALUES)
  sim->value_arr = malloc(sizeof(cheetah_header_field_large_t) * size);
#else
  sim->value_arr = malloc(sizeof(cheetah_header_field_t) * size);
#endif /* CHEETAH_USE_STRING_VALUES */

  RTE_ASSERT(sim->value_arr != NULL);

#ifdef CHEETAH_USE_KEY_AT_WORKER
  sim->key_arr = malloc(sizeof(cheetah_header_field_t) * size);
  RTE_ASSERT(sim->key_arr != NULL);
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  sim->flow_id_arr = malloc(sizeof(cheetah_header_field_small_t) * size);
  RTE_ASSERT(sim->flow_id_arr != NULL);
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

  cheetah_debug("Success!\n");
  cheetah_debug_flush();

  cheetah_debug("Parsing dataset...");
  cheetah_debug_flush();

  stream = fopen(filename, "r");
  RTE_ASSERT(stream != NULL);

  cheetah_debug("Opened...");
  cheetah_debug_flush();

  i = 0;
  while ((scan_output =
              fscanf(stream,

#ifdef CHEETAH_DATASET_USER_VALUES
                     "%*[^,],%*[^,],%*[^,],%*[^,],%[^,],%*[^\n]\n", &value
#elif defined(CHEETAH_DATASET_TPCH)
                     "%d,%d\n", &flow_id, &key
#elif defined(CHEETAH_DATASET_RANDOM_KV)
                     "%d,%d\n", &key, &value
#elif defined(CHEETAH_DATASET_USER_DATA_HASHED)
                     "%*d,%d\n", &value
#else  /* CHEETAH_DATASET_USER_VALUES */
                     "%*[^,],%d,%*d\n", &value
#endif /* CHEETAH_DATASET_USER_VALUES */
                     )) != EOF) {
    sim->row_id_arr[i] = i;

#if defined(CHEETAH_USE_FLOW_ID_AT_WORKER) && defined(CHEETAH_DATASET_TPCH)
    sim->flow_id_arr[i] = (cheetah_header_field_small_t)flow_id;
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER && CHEETAH_DATASET_TPCH */

#if defined(CHEETAH_USE_KEY_AT_WORKER) &&                                      \
    (defined(CHEETAH_DATASET_RANDOM_KV) || defined(CHEETAH_DATASET_TPCH))
    sim->key_arr[i] = (cheetah_header_field_t)key;
#endif /* CHEETAH_USE_KEY_AT_WORKER && (CHEETAH_DATASET_RANDOM_KV ||           \
          CHEETAH_DATASET_TPCH)*/

#ifndef CHEETAH_DATASET_TPCH
#ifdef CHEETAH_USE_64_BIT_VALUES
    sim->value_arr[i] = (cheetah_header_field_large_t)value;
#elif defined(CHEETAH_USE_STRING_VALUES)
    sim->value_arr[i] = strndup(value, CHEETAH_CSV_ENTRY_MAX_LENGTH);
#else
    sim->value_arr[i] = value;
#endif /* CHEETAH_USE_64_BIT_VALUES */
#endif /* CHEETAH_DATASET_TPCH */

    i++;

    RTE_ASSERT(i != size);
  }

  fclose(stream);

  cheetah_debug("Success!\n");
  cheetah_debug_flush();

  sim->entry_count = i;

  return sim;
}

void cheetah_memory_sim_free(struct cheetah_memory_sim *sim) {

#ifdef CHEETAH_USE_STRING_VALUES
  register size_t i;

  for (i = 0; i < sim->entry_count; i++) {
    free(sim->value_arr[i]);
  }
#endif /* CHEETAH_USE_STRING_VALUES */

  free(sim->row_id_arr);
  free(sim->value_arr);

#ifdef CHEETAH_USE_KEY_AT_WORKER
  free(sim->key_arr);
#endif /* CHEETAH_USE_KEY_AT_WORKER */

  free(sim);
}
