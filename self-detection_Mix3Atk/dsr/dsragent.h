/* -*- c++ -*-
   dsragent.h
*/

#ifndef _DSRAgent_h
#define _DSRAgent_h

class DSRAgent;

#include <stdarg.h>

#include <object.h>
#include <agent.h>
#include <trace.h>
#include <packet.h>
#include <dsr-priqueue.h>
#include <mac.h>
#include <mobilenode.h>

#include "path.h"
#include "srpacket.h"
#include "routecache.h"
#include "requesttable.h"
#include "flowstruct.h"

#define BUFFER_CHECK 0.03	// seconds between buffer checks
#define RREQ_JITTER 0.010	// seconds to jitter broadcast route requests
#define SEND_TIMEOUT 30.0	// # seconds a packet can live in sendbuf
#define SEND_BUF_SIZE 64
#define RTREP_HOLDOFF_SIZE 10

#define GRAT_ROUTE_ERROR 0	// tell_addr indicating a grat route err

#define DSR_FILTER_TAP		/* process a packet only once via the tap */

//Define by CRJ
#define HELLO_INTERVAL 1.0
#define NEIGHBOR_VALID_DURATION 4
#define MAX_NEIGHBOR_NUMBER 50
#define MAX_HISTORY_RECORD 600
#define MAX_INVEST_RECORD  100
#define MAX_ROUTE_LEN 16
#define HISTORY_VALID_DURATION 900
#define PDR_THRESHOLD 0.7
#define REC_BLACK_THRESHOLD -0.5
#define REC_WHITE_THRESHOLD 0.5
#define CONFIDENT_THRESHOLD 0.7

class ArpCallbackClass;
struct RtRepHoldoff {
  ID requestor;
  ID requested_dest;
  int best_length;
  int our_length;
};

struct SendBufEntry {
  Time t;			// insertion time
  SRPacket p;
};

struct GratReplyHoldDown {
  Time t;
  Path p;
};

//Help to build a dynamic array by CRJ
typedef struct{
 int *array;
 size_t used;
 size_t size;
} d_array;

//one-hop neighbor info
struct Neighbor_Info {
  int one_hop_neighbor_id;
  int time_left;

  //int two_hop_neighbor[MAX_NEIGHBOR_NUMBER];
};

struct Send_History {
  int dest_id;
  int total_packet;
  int time_left;
  int sr_route[MAX_ROUTE_LEN];
};

struct Rep_Rec {
  int node;
  int rep_pkt;
};

struct Receive_History {
  int src_id;
  int total_packet;
  int time_left; 
  int sr_route[MAX_ROUTE_LEN];
  struct Rep_Rec rep_rec[MAX_NEIGHBOR_NUMBER];
};

struct Trust_Level {
  int node_id;
  float trust;
  float distrust;
};

struct B_W_List {
  int black_node;
  int white_node;
  int Btime_left;
  int Wtime_left;
  int type; //-1: black_node, 1: white_node
  int PR_TP; //1: permanet, 0: temperary
};

class SendBufferTimer : public TimerHandler {
public:
  SendBufferTimer(DSRAgent *a) : TimerHandler() { a_ = a;}
  void expire(Event *e);
protected:
  DSRAgent *a_;
};

//new timer class for hello message broadcast
class HelloTimer : public TimerHandler {
public:
  HelloTimer(DSRAgent *a) : TimerHandler() { a_ = a;}
  void expire(Event *e);
protected:
  DSRAgent *a_;
};

LIST_HEAD(DSRAgent_List, DSRAgent);

class DSRAgent : public Tap, public Agent {
public:

  virtual int command(int argc, const char*const* argv);
  virtual void recv(Packet*, Handler* callback = 0);

  void tap(const Packet *p);
  // tap out all data packets received at this host and promiscously snoop
  // them for interesting tidbits

  void Terminate(void);
  // called at the end of the simulation to purge all packets
  void sendOutBCastPkt(Packet *p);
  
  DSRAgent();
  ~DSRAgent();

int isBlackhole; //is blackhole or not
int isActive; //is active blackhole or not
int isGrayhole;

private:

  Trace *logtarget;
  int off_mac_;
  int off_ll_;
  int off_ip_;
  int off_sr_;

  // will eventually need to handle multiple infs, but this is okay for
  // now 1/28/98 -dam
  ID net_id, MAC_id;		// our IP addr and MAC addr
  NsObject *ll;		        // our link layer output 
  CMUPriQueue *ifq;		// output interface queue
  Mac *mac_;

  // extensions for wired cum wireless sim mode
  MobileNode *node_;
  int diff_subnet(ID dest, ID myid);
  
  // extensions for mobileIP
  NsObject *port_dmux_;    // my port dmux
  
#ifdef DSR_FILTER_TAP
#define TAP_CACHE_SIZE	1024
#define TAP_BITMASK	(TAP_CACHE_SIZE - 1)
  /*
   *  A cache of recently seen packets on the TAP so that I
   *  don't process them over and over again.
   */
  int tap_uid_cache[TAP_CACHE_SIZE];
#endif

  /******** internal state ********/
  RequestTable request_table;
  RouteCache *route_cache;
  SendBufEntry send_buf[SEND_BUF_SIZE];
  SendBufferTimer send_buf_timer;
  int route_request_num;	// number for our next route_request
  int num_heldoff_rt_replies;
  RtRepHoldoff rtrep_holdoff[RTREP_HOLDOFF_SIZE]; // not used 1/27/98
  GratReplyHoldDown grat_hold[RTREP_HOLDOFF_SIZE];
  int grat_hold_victim;

  //Define a variabale in DSRAgent that can access class or structure by CRJ
  HelloTimer hello_brdcst;
  struct Neighbor_Info neighbor_info[MAX_NEIGHBOR_NUMBER];
  struct Send_History send_history[MAX_HISTORY_RECORD];
  struct Receive_History receive_history[MAX_HISTORY_RECORD];
  struct Trust_Level trust_level[MAX_NEIGHBOR_NUMBER];
  struct B_W_List b_w_list[MAX_NEIGHBOR_NUMBER];
  //struct Blacklist blacklist[MAX_NEIGHBOR_NUMBER];
  //struct Attacker_file attacker_file[50];
  //end variable define

  /* for flow state ych 5/2/01 */
  FlowTable flow_table;
  ARSTable  ars_table;

  bool route_error_held; // are we holding a rt err to propagate?
  ID err_from, err_to;	 // data from the last route err sent to us 
  Time route_error_data_time; // time err data was filled in

  /****** internal helper functions ******/

  /* all handle<blah> functions either free or hand off the 
     p.pkt handed to them */
  void handlePktWithoutSR(SRPacket& p, bool retry);
  /* obtain a source route to p's destination and send it off */
  void handlePacketReceipt(SRPacket& p);
  void handleForwarding(SRPacket& p);
  void handleRouteRequest(SRPacket &p);
  /* process a route request that isn't targeted at us */

  /* flow state handle functions ych */
  void handleFlowForwarding(SRPacket &p);
  void handleFlowForwarding(SRPacket &p, int flowidx);
  void handleDefaultForwarding(SRPacket &p);

  bool ignoreRouteRequestp(SRPacket& p);
  // assumes p is a route_request: answers true if it should be ignored.
  // does not update the request table (you have to do that yourself if
  // you want this packet ignored in the future)
  void sendOutPacketWithRoute(SRPacket& p, bool fresh, Time delay = 0.0);
  // take packet and send it out packet must a have a route in it
  // fresh determines whether route is reset first
  // time at which packet is sent is scheduled delay secs in the future
  // pkt.p is freed or handed off
  void sendOutRtReq(SRPacket &p, int max_prop = MAX_SR_LEN);
  // turn p into a route request and launch it, max_prop of request is
  // set as specified
  // p.pkt is freed or handed off
  void getRouteForPacket(SRPacket &p, bool retry);
  /* try to obtain a route for packet
     pkt is freed or handed off as needed, unless in_buffer == true
     in which case they are not touched */
  void acceptRouteReply(SRPacket &p);
  /* - enter the packet's source route into our cache
     - see if any packets are waiting to be sent out with this source route
     - doesn't free the p.pkt */
  void returnSrcRouteToRequestor(SRPacket &p);
  // take the route in p, add us to the end of it and return the
  // route to the sender of p
  // doesn't free p.pkt
  bool replyFromRouteCache(SRPacket &p); 
  /* - see if can reply to this route request from our cache
     if so, do it and return true, otherwise, return false 
     - frees or hands off p.pkt i ff returns true */
  void processUnknownFlowError(SRPacket &p, bool asDefault);
  void processFlowARS(const Packet *packet);
  // same idea as below, but for unknown flow error
  void processBrokenRouteError(SRPacket& p);
  // take the error packet and proccess our part of it.
  // if needed, send the remainder of the errors to the next person
  // doesn't free p.pkt

  void sendUnknownFlow(SRPacket &p, bool asDefault, u_int16_t flowid = 0);

  void xmitFailed(Packet *pkt, const char* reason = "DROP_RTR_MAC_CALLBACK");

  void xmitFlowFailed(Packet *pkt, const char* reason = "DROP_RTR_MAC_CALLBACK");

  /* mark our route cache reflect the failure of the link between
     srh[cur_addr] and srh[next_addr], and then create a route err
     message to send to the orginator of the pkt (srh[0]) 
     p.pkt freed or handed off */
  
  void undeliverablePkt(Packet *p, int mine);
  /* when we've got a packet we can't deliver, what to do with it? 
     frees or hands off p if mine = 1, doesn't hurt it otherwise */

  void dropSendBuff(SRPacket &p);
  // log p as being dropped by the sendbuffer in DSR agent
  
  void stickPacketInSendBuffer(SRPacket& p);
  
  void sendBufferCheck();
  // see if any packets in send buffer need route requests sent out
  // for them, or need to be expired

  void sendRouteShortening(SRPacket &p, int heard_at, int xmit_at);
  // p was overheard at heard_at in it's SR, but we aren't supposed to
  // get it till xmit_at, so all the nodes between heard_at and xmit_at
  // can be elided.  Send originator of p a gratuitous route reply to 
  // tell them this.

  void testinit();
  void trace(char* fmt, ...);

  friend void XmitFailureCallback(Packet *pkt, void *data);
  friend void XmitFlowFailureCallback(Packet *pkt, void *data);
  friend int FilterFailure(Packet *p, void *data);
  friend class SendBufferTimer;
  friend class HelloTimer;

 //This is added associate funtion by CRJ
  bool disturbForwarding(SRPacket &p);
 
 //Functions related to sending history table
  void update_sending_history_time();
  void update_sending_history(SRPacket &p);
  int exist_in_SHT(SRPacket &p); 
  void clear_sending_history(int row_index);
 //Functions related to receiving history table
  void update_receiving_history_time();
  void update_receiving_history(SRPacket &p);
  int exist_in_RHT(SRPacket &p);
//Function related to investigation message
  void send_investigation_request(SRPacket &p);
  void send_investigation_reply(SRPacket &p, int num_of_record);
//Function related to trust evaluation
  void trust_evaluation(SRPacket &p);
  int corresponding_row_SHT(SRPacket &p, int row_IREP);
  void increase_distrust(int index_SHT, int investigator_pos, int lost_Packet);
  int exist_in_TET(int node_id);
  void increase_trust(int index_SHT, int investigator_pos, int received_Packet);
  void increase_trust_prev(SRPacket &p); 

//Function related to blacklist
  void update_BWlist_time();
  void update_blacklist(int node_id, int PR_TP);
  void update_whitelist(int node_id, int PR_TP);
  bool in_blacklist(int node_id);

 //Functions related to HELLO BROADCAST
  void broadcasthello();
  void recv_hello_message(Packet *p);
  void update_neighbor_info_time();
  //end of functions added by CRJ

#if 0
  void scheduleRouteReply(Time t, Packet *new_p);
  // schedule a time to send new_p if we haven't heard a better
  // answer in the mean time.  Do not modify new_p after calling this
  void snoopForRouteReplies(Time t, Packet *p);
  
friend void RouteReplyHoldoffCallback(Node *node, Time time, EventData *data);
#endif //0

  /* the following variables are used to send end-of-sim notices to all objects */
public:
	LIST_ENTRY(DSRAgent) link;
	static DSRAgent_List agthead;
};

#endif // _DSRAgent_h
