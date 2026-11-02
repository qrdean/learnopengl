#include <cstdio>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// int getaddrinfo(const char *node, const char service,
//                 const struct addrinfo *hints, struct addrinfo **res);

int main(int argc, char *argv[]) {
  struct addrinfo hints, *res, *p;
  int status, s;
  char ipstr[INET6_ADDRSTRLEN];

  if (argc != 2) {
    fprintf(stderr, "usage: showip hostname\n");
    return 1;
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // either IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 2;
  }

  printf("ip addresses for %s\n\n", argv[1]);

  for (p = res; p != NULL; p = p->ai_next) {
    void *addr;
    const char *ipver;
    struct sockaddr_in *ipv4;
    struct sockaddr_in6 *ipv6;

    // get pointer to the address itself, different fields in IPv4 & IPV6
    if (p->ai_family == AF_INET) { // IPv4
      ipv4 = (struct sockaddr_in*)p->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = "IPv4";
    } else {
      ipv6 = (struct sockaddr_in6*)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = "IPv6";
    }

    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
    printf("   %s: %s\n", ipver, ipstr);
  }

  s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (s == -1) {
    fprintf(stderr, "creating socket returned: %d\n", s);
    return 2;
  }

  printf("got socket %d\n", s);

  freeaddrinfo(res);
  return 0;
}
