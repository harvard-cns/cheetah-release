#ifndef CHEETAH_PACKET_MANAGER_H
#define CHEETAH_PACKET_MANAGER_H

typedef uint16_t cheetah_header_field_small_t;
typedef uint32_t cheetah_header_field_t;
typedef uint64_t cheetah_header_field_large_t;

struct cheetah_packet {

#ifdef CHEETAH_USE_64_BIT_VALUES
  cheetah_header_field_large_t value;
#else
  cheetah_header_field_t value;
#endif /* CHEETAH_USE_64_BIT_VALUES */

#ifdef CHEETAH_USE_HASH_AT_WORKER
  cheetah_header_field_t hash_val;
#endif /* CHEETAH_USE_HASH_AT_WORKER */

#ifdef CHEETAH_USE_KEY_AT_WORKER
  cheetah_header_field_t key;
#endif /* CHEETAH_USE_KEY_AT_WORKER */

  cheetah_header_field_t row_id;

  cheetah_header_field_small_t flow_id;
};

struct cheetah_packet_stream {
  struct cheetah_packet *packet;
  struct cheetah_packet_stream *next;
};

struct cheetah_packet_stream *cheetah_packet_stream_create(void);

struct cheetah_packet *
cheetah_packet_create(const cheetah_header_field_t row_id,
                      const cheetah_header_field_t value);

void cheetah_packet_free(struct cheetah_packet *pkt);

struct cheetah_packet_stream *
cheetah_packet_stream_add(struct cheetah_packet_stream *const stream,
                          const cheetah_header_field_t row_id,
                          const cheetah_header_field_t value);

void cheetah_packet_stream_send(
    const struct cheetah_connection_state *const state,
    struct cheetah_packet_stream *const pktstream);

#endif /* CHEETAH_PACKET_MANAGER_H */
