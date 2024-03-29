/****************************************************************************
 * apps/netutils/ping/icmp_ping.c
 *
 *   Copyright (C) 2018 Pinecone Inc. All rights reserved.
 *   Author: Guiding Li<liguiding@pinecone.net>
 *
 * Extracted from logic originally written by:
 *
 *   Copyright (C) 2017-2018 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/socket.h>

#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <poll.h>
#include <string.h>
#include <errno.h>

#ifdef CONFIG_LIBC_NETDB
#  include <netdb.h>
#endif

#include <arpa/inet.h>

#include <nuttx/clock.h>
#include <nuttx/net/icmp.h>

#include "netutils/icmp_ping.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ICMP_IOBUFFER_SIZE(x) (sizeof(struct icmp_hdr_s) + (x))

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct rcvdmap_s {
    uint8_t marker;
    clock_t clock;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ping_gethostip
 *
 * Description:
 *   Call gethostbyname() to get the IP address associated with a hostname.
 *
 * Input Parameters
 *   hostname - The host name to use in the nslookup.
 *   ipv4addr - The location to return the IPv4 address.
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno value on failure.
 *
 ****************************************************************************/

static int ping_gethostip(FAR const char *hostname, FAR struct in_addr *dest)
{
#ifdef CONFIG_LIBC_NETDB
  /* Netdb DNS client support is enabled */

  FAR struct addrinfo hint;
  FAR struct addrinfo *info;
  FAR struct sockaddr_in *addr;

  memset(&hint, 0, sizeof(hint));
  hint.ai_family = AF_INET;

  if (getaddrinfo(hostname, NULL, &hint, &info) != OK)
    {
      return ERROR;
    }

  addr = (FAR struct sockaddr_in *)info->ai_addr;
  memcpy(dest, &addr->sin_addr, sizeof(struct in_addr));

  freeaddrinfo(info);
  return OK;

#else /* CONFIG_LIBC_NETDB */
  /* No host name support */

  /* Convert strings to numeric IPv6 address */

  int ret = inet_pton(AF_INET, hostname, dest);

  /* The inet_pton() function returns 1 if the conversion succeeds. It will
   * return 0 if the input is not a valid IPv4 dotted-decimal string or -1
   * with errno set to EAFNOSUPPORT if the address family argument is
   * unsupported.
   */

  return (ret > 0) ? OK : ERROR;

#endif /* CONFIG_LIBC_NETDB */
}

/****************************************************************************
 * Name: icmp_callback
 ****************************************************************************/

static void icmp_callback(FAR struct ping_result_s *result,
                          int code, int extra)
{
  result->code = code;
  result->extra = extra;
  result->info->callback(result);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: icmp_ping
 ****************************************************************************/

void icmp_ping(FAR const struct ping_info_s *info)
{
  struct ping_result_s result, reply;
  struct sockaddr_in destaddr;
  struct sockaddr_in fromaddr;
  struct icmp_hdr_s outhdr;
  FAR struct icmp_hdr_s *inhdr;
  FAR struct rcvdmap_s *maps;
  struct pollfd recvfd;
  FAR uint8_t *iobuffer;
  FAR uint8_t *ptr;
  uint16_t timeout;
  int32_t elapsed;
  int32_t interval;
  clock_t kickoff;
  socklen_t addrlen;
  ssize_t nsent;
  ssize_t nrecvd;
  int sockfd;
  int ret;
  int ch;
  int i;

  /* Set the clock as the seed to pseudo-random number generator */

  srand(clock());

  /* Initialize result structure */

  memset(&result, 0, sizeof(result));
  result.info = info;
  result.id = rand();
  result.outsize = ICMP_IOBUFFER_SIZE(info->datalen);
  if (ping_gethostip(info->hostname, &result.dest) < 0)
    {
      icmp_callback(&result, ICMP_E_HOSTIP, 0);
      return;
    }

  maps = (struct rcvdmap_s *)malloc(info->count * sizeof(struct rcvdmap_s));
  if (maps == NULL)
    {
      icmp_callback(&result, ICMP_E_MEMORY, 0);
      return;
    }

  /* Allocate memory to hold ping buffer */

  iobuffer = (FAR uint8_t *)malloc(result.outsize);
  if (iobuffer == NULL)
    {
      free(maps);
      icmp_callback(&result, ICMP_E_MEMORY, 0);
      return;
    }

  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
  if (sockfd < 0)
    {
      icmp_callback(&result, ICMP_E_SOCKET, errno);
      free(maps);
      free(iobuffer);
      return;
    }

  kickoff = clock();

  memset(&destaddr, 0, sizeof(struct sockaddr_in));
  destaddr.sin_family      = AF_INET;
  destaddr.sin_port        = 0;
  destaddr.sin_addr.s_addr = result.dest.s_addr;

  memset(&outhdr, 0, sizeof(struct icmp_hdr_s));
  outhdr.type              = ICMP_ECHO_REQUEST;
  outhdr.id                = htons(result.id);
  outhdr.seqno             = htons(result.seqno);

  icmp_callback(&result, ICMP_I_BEGIN, 0);

  while (result.nrequests < info->count)
    {
      /* Copy the ICMP header into the I/O buffer */

      memcpy(iobuffer, &outhdr, sizeof(struct icmp_hdr_s));

      /* Add some easily verifiable payload data */

      ptr = &iobuffer[sizeof(struct icmp_hdr_s)];
      ch  = 0x20;

      for (i = 0; i < info->datalen; i++)
        {
          *ptr++ = ch;
          if (++ch > 0x7e)
            {
              ch = 0x20;
            }
        }

      maps[result.seqno].clock = clock();
      maps[result.seqno].marker = 0;
      nsent = sendto(sockfd, iobuffer, result.outsize, 0,
                     (FAR struct sockaddr *)&destaddr,
                     sizeof(struct sockaddr_in));
      if (nsent < 0)
        {
          icmp_callback(&result, ICMP_E_SENDTO, errno);
          goto done;
        }
      else if (nsent != result.outsize)
        {
          icmp_callback(&result, ICMP_E_SENDSMALL, nsent);
          goto done;
        }

      result.nrequests++;

      elapsed = 0;
      timeout = info->timeout;

      while (timeout >= elapsed)
        {
          recvfd.fd       = sockfd;
          recvfd.events   = POLLIN;
          recvfd.revents  = 0;

          ret = poll(&recvfd, 1, timeout - elapsed);

          elapsed = (unsigned int)TICK2MSEC(clock() - maps[result.seqno].clock);

          if (ret < 0)
            {
              icmp_callback(&result, ICMP_E_POLL, errno);
              goto done;
            }
          else if (ret == 0)
            {
              if (maps[result.seqno].marker)
                {
                  break;
                }
              icmp_callback(&result, ICMP_W_TIMEOUT, timeout);
              continue;
            }

          /* Get the ICMP response (ignoring the sender) */

          addrlen = sizeof(struct sockaddr_in);
          nrecvd  = recvfrom(sockfd, iobuffer, result.outsize, 0,
                             (FAR struct sockaddr *)&fromaddr, &addrlen);
          if (nrecvd < 0)
            {
              icmp_callback(&result, ICMP_E_RECVFROM, errno);
              goto done;
            }
          else if (nrecvd < sizeof(struct icmp_hdr_s))
            {
              icmp_callback(&result, ICMP_E_RECVSMALL, nrecvd);
              goto done;
            }

          inhdr     = (FAR struct icmp_hdr_s *)iobuffer;
          interval  = (unsigned int)TICK2MSEC(clock() - maps[ntohs(inhdr->seqno)].clock);

          if (inhdr->type == ICMP_ECHO_REPLY)
            {
              if (ntohs(inhdr->id) != result.id)
                {
                  icmp_callback(&result, ICMP_W_IDDIFF, ntohs(inhdr->id));
                }
              else if (ntohs(inhdr->seqno) > result.seqno)
                {
                  icmp_callback(&result, ICMP_W_SEQNOBIG, ntohs(inhdr->seqno));
                }
              else
                {
                  bool verified = true;

                  if (maps[ntohs(inhdr->seqno)].marker)
                    {
                      memcpy(&reply, &result, sizeof(reply));
                      reply.seqno = ntohs(inhdr->seqno);
                      icmp_callback(&reply, ICMP_I_PKTDUP, interval);
                    }
                  else if (ntohs(inhdr->seqno) < result.seqno)
                    {
                      icmp_callback(&result, ICMP_W_SEQNOSMALL, ntohs(inhdr->seqno));
                      memcpy(&reply, &result, sizeof(reply));
                      reply.seqno = ntohs(inhdr->seqno);
                      icmp_callback(&reply, ICMP_I_ROUNDTRIP, interval);
                    }
                  else
                    {
                      timeout = info->delay;
                      icmp_callback(&result, ICMP_I_ROUNDTRIP, interval);
                    }

                  maps[ntohs(inhdr->seqno)].marker = 1;

                  /* Verify the payload data */

                  if (nrecvd != result.outsize)
                    {
                      icmp_callback(&result, ICMP_W_RECVBIG, nrecvd);
                      verified = false;
                    }
                  else
                    {
                      ptr = &iobuffer[sizeof(struct icmp_hdr_s)];
                      ch  = 0x20;

                      for (i = 0; i < info->datalen; i++, ptr++)
                        {
                          if (*ptr != ch)
                            {
                              icmp_callback(&result, ICMP_W_DATADIFF, 0);
                              verified = false;
                              break;
                            }

                          if (++ch > 0x7e)
                            {
                              ch = 0x20;
                            }
                        }
                    }

                  /* Only count the number of good replies */

                  if (verified)
                    {
                      result.nreplies++;
                    }
                }
            }
          else
            {
              icmp_callback(&result, ICMP_W_TYPE, inhdr->type);
            }
        }

      outhdr.seqno = htons(++result.seqno);
    }

done:
  icmp_callback(&result, ICMP_I_FINISH, TICK2MSEC(clock() - kickoff));
  close(sockfd);
  free(maps);
  free(iobuffer);
}
