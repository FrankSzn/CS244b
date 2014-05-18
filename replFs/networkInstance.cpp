/*
 * networkInstance.cpp
 *
 *  Created on: May 17, 2014
 *      Author: songhan
 */

#include "networkInstance.h"

Network::Network() {
//  myAddr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
//  if (!myAddr) {
//    ERROR("Error allocating sockaddr variable");
//  }

  struct sockaddr_in nullAddr;
  struct sockaddr_in *thisHost;
  char buf[128];
  int reuse;
  u_char ttl;
  struct ip_mreq mreq;

  gethostname(buf, sizeof(buf));
  if ((thisHost = resolveHost(buf)) == (struct sockaddr_in *) NULL)
    ERROR("who am I?");
  bcopy((caddr_t) thisHost, (caddr_t) (&myAddr), sizeof(struct sockaddr_in));

  mySocket = socket(AF_INET, SOCK_DGRAM, 0);
  if (mySocket < 0)
    ERROR("can't get socket");

  /* SO_REUSEADDR allows more than one binding to the same
   socket - you cannot have more than one player on one
   machine without this */
  reuse = 1;
  if (setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))
      < 0) {
    ERROR("setsockopt failed (SO_REUSEADDR)");
  }

  nullAddr.sin_family = AF_INET;
  nullAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  nullAddr.sin_port = PORT;
  if (bind(mySocket, (struct sockaddr *) &nullAddr, sizeof(nullAddr)) < 0)
    ERROR("netInit binding");

  /* Multicast TTL:
   0 restricted to the same host
   1 restricted to the same subnet
   32 restricted to the same site
   64 restricted to the same region
   128 restricted to the same continent
   255 unrestricted

   DO NOT use a value > 32. If possible, use a value of 1 when
   testing.
   */

  ttl = 1;
  if (setsockopt(mySocket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl))
      < 0) {
    ERROR("setsockopt failed (IP_MULTICAST_TTL)");
  }

  /* join the multicast group */
  mreq.imr_multiaddr.s_addr = htonl(GROUP);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if (setsockopt(mySocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq,
                 sizeof(mreq)) < 0) {
    ERROR("setsockopt failed (IP_ADD_MEMBERSHIP)");
  }


  /* Get the multi-cast address ready to use in SendData()
   calls. */
  memcpy(&myAddr, &nullAddr, sizeof(struct sockaddr_in));
  groupAddr.sin_addr.s_addr = htonl(GROUP);

}

Network::~Network() {
}

/*
 * Resolve the specified host name into an internet address.  The "name" may
 * be either a character string name, or an address in the form a.b.c.d where
 * the pieces are octal, decimal, or hex numbers.  Returns a pointer to a
 * sockaddr_in struct (note this structure is statically allocated and must
 * be copied), or NULL if the name is unknown.
 */

struct sockaddr_in * Network::resolveHost(register char *name) {
  register struct hostent *fhost;
  struct in_addr fadd;
  static struct sockaddr_in sa;

  if ((fhost = gethostbyname(name)) != NULL) {
    sa.sin_family = fhost->h_addrtype;
    sa.sin_port = 0;
    bcopy(fhost->h_addr, &sa.sin_addr, fhost->h_length);
  } else {
    fadd.s_addr = inet_addr(name);
    if (fadd.s_addr != -1) {
      sa.sin_family = AF_INET; /* grot */
      sa.sin_port = 0;
      sa.sin_addr.s_addr = fadd.s_addr;
    } else
    return(NULL);
  }
  return (&sa);
}