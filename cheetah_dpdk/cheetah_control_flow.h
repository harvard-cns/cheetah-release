#ifndef CHEETAH_CONTROL_FLOW_H
#define CHEETAH_CONTROL_FLOW_H

#define BATCH_SIZE 32
#define RECEIVER_BATCH_SIZE 32

/* #define CHEETAH_CONTROL_FLOW_WORKER_DEBUG */
/* #define CHEETAH_CONTROL_FLOW_MASTER_DEBUG */

int cheetah_control_flow_worker(int argc, char **const argv);

int cheetah_control_flow_master(int argc, char **const argv);

#endif /* CHEETAH_CONTROL_FLOW_H */
