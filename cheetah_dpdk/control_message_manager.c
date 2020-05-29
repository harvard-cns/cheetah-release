#include "cheetah_dpdk.h"

struct cheetah_control_connection_state *
cheetah_control_connection_state_create(const uint16_t port) {

  struct cheetah_control_connection_state *state;

  state = malloc(sizeof(struct cheetah_control_connection_state));

  state->server_address_info = malloc(sizeof(struct sockaddr_in));
  state->client_address_info = malloc(sizeof(struct sockaddr_in));

  memset(state->server_address_info, 0, sizeof(struct sockaddr_in));
  memset(state->client_address_info, 0, sizeof(struct sockaddr_in));

  state->server_address_info->sin_family = AF_INET;
  state->server_address_info->sin_addr.s_addr = INADDR_ANY;
  state->server_address_info->sin_port = htons(port);

  state->port = port;
  state->socket_file_descriptor = -1;

  return state;
}

void cheetah_control_connection_state_free(
    struct cheetah_control_connection_state *state) {

  close(state->socket_file_descriptor);

  free(state->server_address_info);
  free(state->client_address_info);
  free(state);
}
