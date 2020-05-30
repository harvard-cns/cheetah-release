#ifndef CHEETAH_RESOURCE_MANAGER_H
#define CHEETAH_RESOURCE_MANAGER_H

typedef uintmax_t cheetah_resource_limit_t;

void cheetah_set_memory_limit(const cheetah_resource_limit_t lim);

void cheetah_set_cpu_core_limit(const cheetah_resource_limit_t lim);

#endif /* CHEETAH_RESOURCE_MANAGER_H */
