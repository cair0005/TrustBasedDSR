/* -*- c++ -*-
   srpacket.h
*/

#ifndef _SRPACKET_H_
#define _SRPACKET_H_

#include <packet.h>
#include "hdr_sr.h"

#include "path.h"

struct SRPacket {

  ID dest;
  ID src;
  Packet *pkt;			/* the inner NS packet */
  Path route;
  SRPacket(Packet *p, struct hdr_sr *srh) : pkt(p), route(srh) {}
  SRPacket() : pkt(NULL) {}
};

#endif  //_SRPACKET_H_
