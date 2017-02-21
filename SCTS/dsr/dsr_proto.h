 /* -*- c++ -*- 
   dsr_proto.h
   the DSR routing protocol agent
*/

#ifndef _dsr_proto_h
#define _dsr_proto_h

#include <packet.h>
#include <object.h>
#include <agent.h>

#include "path.h"
#include "routecache.h"

class DSRProto : public Agent {
public:
  DSRProto();
  void recv(Packet*, Handler* callback = 0);
  int command(int argc, const char*const* argv);
  void noRouteForPacket(Packet *p);

private:
  Trace *tracetarget;
  RouteCache *routecache;
  void testinit();
  ID net_id;
  ID mac_id;
};

#endif //_dsr_proto_h
