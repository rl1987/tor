#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>

int
connect(int socket, const struct sockaddr *address, socklen_t address_len)
{
  char ipstr[INET6_ADDRSTRLEN];
  memset(ipstr, 0, sizeof(ipstr));

  if (address->sa_family == AF_INET) {
    const struct sockaddr_in *ipv4_sockaddr =
      (const struct sockaddr_in *)address;
    inet_ntop(address->sa_family, &ipv4_sockaddr->sin_addr,
              ipstr, INET6_ADDRSTRLEN);
  } else if (address->sa_family == AF_INET6) {
    const struct sockaddr_in6 *ipv6_sockaddr =
      (const struct sockaddr_in6 *)address;
    inet_ntop(address->sa_family, &ipv6_sockaddr->sin6_addr,
              ipstr, INET6_ADDRSTRLEN);
  }

  fprintf(stderr, "connect() %s\n", ipstr);

  return 0;
}

ssize_t
send(int socket, const void *buffer, size_t length, int flags)
{
  const uint8_t *buf = (const uint8_t *)buffer;

  for (size_t i = 0; i < length; i++) {
    uint8_t b = *(buf + i);
    fprintf(stderr, "%02X ", b);
  }

  fprintf(stderr, "\n");

  return length;
}

ssize_t
recv(int socket, void *buffer, size_t length, int flags)
{
  char *buf = (char *)buffer;

  fprintf(stderr, "recv %zu\n", length);

  for (size_t i = 0; i < length; i++) {
    unsigned int in = 0;
    scanf("%02X", &in);
    if (in < 0xff) {
      uint8_t in8 = (uint8_t)in;
      *(buf + i) = in8;
    }
  }

  fprintf(stderr, "!\n");

  return length;
}

