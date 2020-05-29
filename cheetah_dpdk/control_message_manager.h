#ifndef CHEETAH_CONTROL_MESSAGE_MANAGER_H
#define CHEETAH_CONTROL_MESSAGE_MANAGER_H

struct cheetah_control_connection_state {
  int socket_file_descriptor;
  uint16_t port;
  struct sockaddr_in *server_address_info;
  struct sockaddr_in *client_address_info;
};

struct cheetah_control_connection_state *
cheetah_control_connection_state_create(const uint16_t port);

void cheetah_control_connection_state_free(
    struct cheetah_control_connection_state *state);

#endif /* CHEETAH_CONTROL_MESSAGE_MANAGER_H */
