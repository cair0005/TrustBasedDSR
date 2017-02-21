/* 
   add_sr.cc
   add a compiled constant source route to a packet.
   for testing purposes  
   */

#include <packet.h>
#include <ip.h>
#include "hdr_sr.h"

#include <connector.h>

class AddSR : public Connector {
public:
  void recv(Packet*, Handler* callback = 0);
  AddSR();

private:
  int off_ip_;
  int off_sr_;
  int off_ll_;
  int off_mac_;
};

static class AddSRClass : public TclClass {
public:
  AddSRClass() : TclClass("Connector/AddSR") {}
  TclObject* create(int, const char*const*) {
    return (new AddSR);
  }
} class_addsr;

AddSR::AddSR()
{
  bind("off_sr_", &off_sr_);
  bind("off_ll_", &off_ll_);
  bind("off_mac_", &off_mac_);
  bind("off_ip_", &off_ip_);
}

void
AddSR::recv(Packet* p, Handler* )
{
  hdr_sr *srh =  hdr_sr::access(p);

  srh->route_request() = 0;
  srh->num_addrs() = 0;
  srh->cur_addr() = 0;
  srh->valid() = 1;
  srh->append_addr( 1<<8, NS_AF_INET );
  srh->append_addr( 2<<8, NS_AF_INET );
  srh->append_addr( 3<<8, NS_AF_INET );
  printf(" added sr %s\n",srh->dump());
  send(p,0);
}
