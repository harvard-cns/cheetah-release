#ifndef CHEETAH_MEMORY_SIM_H
#define CHEETAH_MEMORY_SIM_H

struct cheetah_memory_sim {
  cheetah_header_field_t *row_id_arr;

#ifdef CHEETAH_USE_64_BIT_VALUES
  cheetah_header_field_large_t *value_arr;
#elif defined(CHEETAH_USE_STRING_VALUES)
  char **value_arr;
#else
  cheetah_header_field_t *value_arr;
#endif /* CHEETAH_USE_64_BIT_VALUES */

#ifdef CHEETAH_USE_KEY_AT_WORKER
  cheetah_header_field_t *key_arr;
#endif /* CHEETAH_USE_KEY_AT_WORKER */

#ifdef CHEETAH_USE_FLOW_ID_AT_WORKER
  cheetah_header_field_small_t *flow_id_arr;
#endif /* CHEETAH_USE_FLOW_ID_AT_WORKER */

  size_t entry_count;
  size_t array_count;
};

struct cheetah_memory_sim *cheetah_memory_sim_create(const char *filename,
                                                     const size_t size);

void cheetah_memory_sim_free(struct cheetah_memory_sim *sim);
#endif /* CHEETAH_MEMORY_SIM_H */
