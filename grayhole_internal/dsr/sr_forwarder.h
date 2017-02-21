/* -*- c++ -*-  
   sr_forwarder.h
   source route forwarder
*/

#ifndef _sr_forwarder_
#define _sr_forwarder_

#include <object.h>
#include <trace.h>

#include "dsr_proto.h"
#include "requesttable.h"
#include "routecache.h"

/* a source route forwarding object takes packets and if it has a SR
routing header it forwards the packets into target_ according to the
header.  Otherwise, it gives the packet to the noroute_ object in
hopes that it knows what to do with it. */

class SRForwarder : public NsObject {
public:
  SRForwarder();

protected:
  virtual int command(int argc, const char*const* argv);
  virtual void recv(Packet*, Handler* callback = 0);

  NsObject* target_;		/* where to send forwarded pkts */
  DSRProto* route_agent_;	        /* where to send unforwardable pkts */
  RouteCache* routecache_;	/* the routecache */

private:
  void handlePktWithoutSR(Packet *p);
  Trace *tracetarget;
  int off_mac_;
  int off_ll_;
  int off_ip_;
  int off_sr_;
};
#endif //_sr_forwarder_
