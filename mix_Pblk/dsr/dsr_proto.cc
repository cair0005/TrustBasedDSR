 /* -*- c++ -*- 
   dsr_proto.cc
   the DSR routing protocol agent  
   */

#include <packet.h>
#include <object.h>
#include <agent.h>
#include <trace.h>
#include <ip.h>
#include "routecache.h"
#include "dsr_proto.h"

static class DSRProtoClass : public TclClass {
public:
        DSRProtoClass() : TclClass("Agent/DSRProto") {}
        TclObject* create(int, const char*const*) {
                return (new DSRProto);
        }
} class_DSRProto;


DSRProto::DSRProto() : Agent(PT_DSR)
{
  // dst_ = (IP_BROADCAST << 8) | RT_PORT;
  dst_.addr_ = IP_BROADCAST;
  dst_.port_ = RT_PORT;
}


void
DSRProto::recv(Packet* , Handler* )
{
  
}

void
DSRProto::testinit()
{
  //struct hdr_sr hsr;
  hdr_sr hsr;
  
  if (net_id == ID(1,::IP))
    {
      printf("adding route to 1\n");
      hsr.init();
      hsr.append_addr( 1<<8, NS_AF_INET );
      hsr.append_addr( 2<<8, NS_AF_INET );
      hsr.append_addr( 3<<8, NS_AF_INET );
      hsr.append_addr( 4<<8, NS_AF_INET );
      
      routecache->addRoute(Path(hsr.addrs(), hsr.num_addrs()), 0.0, ID(1,::IP));
    }
  
  if (net_id == ID(3,::IP))
    {
      printf("adding route to 3\n");
      hsr.init();
      hsr.append_addr( 3<<8, NS_AF_INET );
      hsr.append_addr( 2<<8, NS_AF_INET );
      hsr.append_addr( 1<<8, NS_AF_INET );
      
      routecache->addRoute(Path(hsr.addrs(), hsr.num_addrs()), 0.0, ID(3,::IP));
    }
}


int
DSRProto::command(int argc, const char*const* argv)
{
  if(argc == 2) 
    {
      if (strcasecmp(argv[1], "testinit") == 0)
	{
	  testinit();
	  return TCL_OK;
	}
    }

  if(argc == 3) 
    {
      if(strcasecmp(argv[1], "tracetarget") == 0) {
	tracetarget = (Trace*) TclObject::lookup(argv[2]);
	if(tracetarget == 0)
	  return TCL_ERROR;
	return TCL_OK;
      }
      else if(strcasecmp(argv[1], "routecache") == 0) {
	routecache = (RouteCache*) TclObject::lookup(argv[2]);
	if(routecache == 0)
	  return TCL_ERROR;
	return TCL_OK;
      }
      else if (strcasecmp(argv[1], "ip-addr") == 0) {
	net_id = ID(atoi(argv[2]), ::IP);
	return TCL_OK;
      }
      else if(strcasecmp(argv[1], "mac-addr") == 0) {
	mac_id = ID(atoi(argv[2]), ::MAC);
      return TCL_OK;

      }
    }
  return Agent::command(argc, argv);
}

void 
DSRProto::noRouteForPacket(Packet *)
{

}
