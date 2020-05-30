#ifndef CHEETAH_CONTROL_MESSAGE_H
#define CHEETAH_CONTROL_MESSAGE_H

#define CHEETAH_CONTROL_MESSAGE_QUERY_FILTER "FILT"
#define CHEETAH_CONTROL_MESSAGE_QUERY_TOPN "TOPN"
#define CHEETAH_CONTROL_MESSAGE_QUERY_DISTINCT "DIST"
#define CHEETAH_CONTROL_MESSAGE_QUERY_GROUPBY "GRBY"
#define CHEETAH_CONTROL_MESSAGE_QUERY_JOIN "JOIN"
#define CHEETAH_CONTROL_MESSAGE_ACK "ACK"

#define CHEETAH_CONTROL_MESSAGE_ID_UNDEFINED 0
#define CHEETAH_CONTROL_MESSAGE_ID_FILTER 1
#define CHEETAH_CONTROL_MESSAGE_ID_TOPN 2
#define CHEETAH_CONTROL_MESSAGE_ID_DISTINCT 3
#define CHEETAH_CONTROL_MESSAGE_ID_ACK 4

#define CHEETAH_CONTROL_MESSAGE_MAX_SIZE 5

void cheetah_control_message_offload_query(const char *const query_type);

int cheetah_control_message_await_query(const int worker_id);

void cheetah_control_message_send_ack(void);

void cheetah_control_message_receive_acks(int *pn_workers);

#endif /* CHEETAH_CONTROL_MESSAGE_H */
