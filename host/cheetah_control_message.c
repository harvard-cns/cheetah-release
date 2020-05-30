#include "cheetah_dpdk.h"

static int cheetah_control_message_receive(const char *const ip,
                                           const char *port,
                                           const uint8_t non_blocking) {
  int socket_file_descriptor;
  struct addrinfo hints;
  struct addrinfo *server_information;
  struct addrinfo *addr_iterator;
  char recv_buffer[CHEETAH_CONTROL_MESSAGE_BUFFER_LENGTH];
  socklen_t address_length;
  int return_placeholder;
  struct sockaddr_storage sender_address;
  int bytes_received;
  int flags;

  cheetah_unused_var(return_placeholder);
  cheetah_unused_var(bytes_received);

  socket_file_descriptor = -1;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  return_placeholder = getaddrinfo(ip, port, &hints, &server_information);

  RTE_ASSERT(return_placeholder == 0);

  for (addr_iterator = server_information; addr_iterator != NULL;
       addr_iterator = addr_iterator->ai_next) {
    socket_file_descriptor =
        socket(addr_iterator->ai_family, addr_iterator->ai_socktype,
               addr_iterator->ai_protocol);

    if (socket_file_descriptor != -1) {
      return_placeholder = bind(socket_file_descriptor, addr_iterator->ai_addr,
                                addr_iterator->ai_addrlen);

      if (return_placeholder != -1) {
        break;
      }
    }
  }

  RTE_ASSERT(addr_iterator != NULL);

  freeaddrinfo(server_information);

  if (non_blocking) {
    flags = fcntl(socket_file_descriptor, F_GETFL);
    RTE_ASSERT(flags != -1);

    return_placeholder =
        fcntl(socket_file_descriptor, F_SETFL, flags | O_NONBLOCK);
    RTE_ASSERT(return_placeholder != -1);
  }

  address_length = sizeof(struct sockaddr_storage);
  bytes_received =
      recvfrom(socket_file_descriptor, recv_buffer,
               CHEETAH_CONTROL_MESSAGE_BUFFER_LENGTH - 1, 0,
               (struct sockaddr *)&sender_address, &address_length);

  if (!non_blocking) {
    RTE_ASSERT(bytes_received != -1);
  } else {
    if (bytes_received == -1) {
      return -1;
    }
  }

  if (strncmp(CHEETAH_CONTROL_MESSAGE_QUERY_FILTER, recv_buffer,
              bytes_received) == 0) {
    return CHEETAH_CONTROL_MESSAGE_ID_FILTER;
  }

  if (strncmp(CHEETAH_CONTROL_MESSAGE_QUERY_TOPN, recv_buffer,
              bytes_received) == 0) {
    return CHEETAH_CONTROL_MESSAGE_ID_TOPN;
  }

  if (strncmp(CHEETAH_CONTROL_MESSAGE_QUERY_DISTINCT, recv_buffer,
              bytes_received) == 0) {
    return CHEETAH_CONTROL_MESSAGE_ID_DISTINCT;
  }

  if (strncmp(CHEETAH_CONTROL_MESSAGE_ACK, recv_buffer, bytes_received) == 0) {
    return CHEETAH_CONTROL_MESSAGE_ID_ACK;
  }

  return CHEETAH_CONTROL_MESSAGE_ID_UNDEFINED;
}

static void cheetah_control_message_send(const char *const ip,
                                         const char *const port,
                                         const char *const message,
                                         const size_t message_len) {

  int socket_file_descriptor;
  struct addrinfo hints;
  struct addrinfo *server_information;
  struct addrinfo *addr_iterator;
  int return_placeholder;
  int bytes_sent;

  cheetah_unused_var(return_placeholder);
  cheetah_unused_var(bytes_sent);

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  cheetah_debug("Sending control message to %s...", ip);
  cheetah_debug_flush();

  return_placeholder = getaddrinfo(ip, port, &hints, &server_information);
  RTE_ASSERT(return_placeholder != 0);

  for (addr_iterator = server_information; addr_iterator != NULL;
       addr_iterator = addr_iterator->ai_next) {
    socket_file_descriptor =
        socket(addr_iterator->ai_family, addr_iterator->ai_socktype,
               addr_iterator->ai_protocol);

    if (socket_file_descriptor != -1) {
      break;
    }
  }

  RTE_ASSERT(addr_iterator != NULL);

  bytes_sent = sendto(socket_file_descriptor, message, message_len, 0,
                      addr_iterator->ai_addr, addr_iterator->ai_addrlen);
  RTE_ASSERT(bytes_sent != -1);

  cheetah_debug("Done.\n");
  cheetah_debug_flush();

  freeaddrinfo(server_information);
  close(socket_file_descriptor);
}

void cheetah_control_message_offload_query(const char *const query_type) {
  cheetah_control_message_send(
      CHEETAH_CONTROL_MESSAGE_DESTINATION_HOST_1, CHEETAH_CONTROL_MESSAGE_PORT,
      query_type, strnlen(query_type, CHEETAH_CONTROL_MESSAGE_MAX_SIZE));

#ifdef CHEETAH_MULTIPLE_NODES_SAME_IP
  cheetah_control_message_send(
      CHEETAH_CONTROL_MESSAGE_DESTINATION_HOST_1,
      CHEETAH_CONTROL_MESSAGE_PORT_2, query_type,
      strnlen(query_type, CHEETAH_CONTROL_MESSAGE_MAX_SIZE));
#elif defined(CHEETAH_MULTIPLE_NODES)
  cheetah_control_message_send(
      CHEETAH_CONTROL_MESSAGE_DESTINATION_HOST_2, CHEETAH_CONTROL_MESSAGE_PORT,
      query_type, strnlen(query_type, CHEETAH_CONTROL_MESSAGE_MAX_SIZE));

  cheetah_control_message_send(
      CHEETAH_CONTROL_MESSAGE_DESTINATION_HOST_3, CHEETAH_CONTROL_MESSAGE_PORT,
      query_type, strnlen(query_type, CHEETAH_CONTROL_MESSAGE_MAX_SIZE));

  cheetah_control_message_send(
      CHEETAH_CONTROL_MESSAGE_DESTINATION_HOST_4, CHEETAH_CONTROL_MESSAGE_PORT,
      query_type, strnlen(query_type, CHEETAH_CONTROL_MESSAGE_MAX_SIZE));

  cheetah_control_message_send(
      CHEETAH_CONTROL_MESSAGE_DESTINATION_HOST_5, CHEETAH_CONTROL_MESSAGE_PORT,
      query_type, strnlen(query_type, CHEETAH_CONTROL_MESSAGE_MAX_SIZE));
#endif /* CHEETAH_MULTIPLE_NODES*/
}

int cheetah_control_message_await_query(const int worker_id) {
  RTE_ASSERT(worker_id > 0);

  if (worker_id == 1) {
    return cheetah_control_message_receive(
        CHEETAH_CONTROL_MESSAGE_DESTINATION_HOST_1,
        CHEETAH_CONTROL_MESSAGE_PORT, 0);
  }

#ifdef CHEETAH_MULTIPLE_NODES_SAME_IP
  if (worker_id == 2) {
    return cheetah_control_message_receive(
        CHEETAH_CONTROL_MESSAGE_DESTINATION_HOST_1,
        CHEETAH_CONTROL_MESSAGE_PORT_2, 0);
  }

#elif defined(CHEETAH_MULTIPLE_NODES)
  if (worker_id == 2) {
    return cheetah_control_message_receive(
        CHEETAH_CONTROL_MESSAGE_DESTINATION_HOST_2,
        CHEETAH_CONTROL_MESSAGE_PORT, 0);
  }

  if (worker_id == 3) {
    return cheetah_control_message_receive(
        CHEETAH_CONTROL_MESSAGE_DESTINATION_HOST_3,
        CHEETAH_CONTROL_MESSAGE_PORT, 0);
  }

  if (worker_id == 4) {
    return cheetah_control_message_receive(
        CHEETAH_CONTROL_MESSAGE_DESTINATION_HOST_4,
        CHEETAH_CONTROL_MESSAGE_PORT, 0);
  }

  if (worker_id == 5) {
    return cheetah_control_message_receive(
        CHEETAH_CONTROL_MESSAGE_DESTINATION_HOST_5,
        CHEETAH_CONTROL_MESSAGE_PORT, 0);
  }
#endif /* CHEETAH_MULTIPLE_NODES */

  RTE_ASSERT(0);
  return -1;
}

void cheetah_control_message_send_ack(void) {
  cheetah_control_message_send(
      CHEETAH_CONTROL_MESSAGE_MASTER_IP, CHEETAH_CONTROL_MESSAGE_PORT,
      CHEETAH_CONTROL_MESSAGE_ACK,
      strnlen(CHEETAH_CONTROL_MESSAGE_ACK, CHEETAH_CONTROL_MESSAGE_MAX_SIZE));
}

void cheetah_control_message_receive_acks(int *pn_workers) {
  int message_id;

  cheetah_unused_var(message_id);

  RTE_ASSERT(*pn_workers > 0);

  message_id = cheetah_control_message_receive(
      CHEETAH_CONTROL_MESSAGE_MASTER_IP, CHEETAH_CONTROL_MESSAGE_PORT, 0);

  if (message_id != -1) {
    RTE_ASSERT(message_id == CHEETAH_CONTROL_MESSAGE_ID_ACK);
    *pn_workers -= 1;
  }
}
