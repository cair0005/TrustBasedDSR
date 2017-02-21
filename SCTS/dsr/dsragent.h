/*
 * dsragent.h
 * Copyright (C) 2000 by the University of Southern California
 * $Id: dsragent.h,v 1.10 2005/08/25 18:58:04 johnh Exp $
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * The copyright of this module includes the following
 * linking-with-specific-other-licenses addition:
 *
 * In addition, as a special exception, the copyright holders of
 * this module give you permission to combine (via static or
 * dynamic linking) this module with free software programs or
 * libraries that are released under the GNU LGPL and with code
 * included in the standard release of ns-2 under the Apache 2.0
 * license or under otherwise-compatible licenses with advertising
 * requirements (or modified versions of such code, with unchanged
 * license).  You may copy and distribute such a system following the
 * terms of the GNU GPL for this module and the licenses of the
 * other code concerned, provided that you include the source code of
 * that other code when and as the GNU GPL requires distribution of
 * source code.
 *
 * Note that people who make modified versions of this module
 * are not obligated to grant this special exception for their
 * modified versions; it is their choice whether to do so.  The GNU
 * General Public License gives permission to release a modified
 * version without this exception; this exception also makes it
 * possible to release a modified version which carries forward this
 * exception.
 *
 */
// Other copyrights might apply to parts of this software and are so noted when applicable.
//
// Ported from CMU/Monarch's code, appropriate copyright applies.  
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
#define CONFIDENT_THRESHOLD 0.9

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

struct Rec_Info {
  int node_type[MAX_NEIGHBOR_NUMBER]; 
  int time_left; 
};

struct Num_Rec_Info {
  int row_var[MAX_NEIGHBOR_NUMBER];
  int col_var[MAX_NEIGHBOR_NUMBER];
  int R_var;
  int C_var;
  int var;
  int rating;
};

struct Last_Rec_Info {
  int R_var;
  int C_var;
  int var;
  int rating;
};

struct Temp_Rvar {
  int var;
  double result;
};

struct Temp_Cvar {
  int var;
  double result;
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

//Class for pesurdo inverse calculation by CRJ
class  ginv
{
private: 
 int m, n, ka;
 double  **a, **u, **v, **aa, eps;
 double  *s, *e, *w, fg[2], cs[2];
public:
 ginv (int mm, int nn)
 {
  int i;
  m = mm; n= nn;
  a = new double*[m];  
  for (i=0; i<m; i++) a[i] = new double[n];
  u = new double*[m];
  for (i=0; i<m; i++) u[i] = new double[m];
  v = new double*[n];
  for (i=0; i<n; i++) v[i] = new double[n];
  aa = new double*[n];
  for (i=0; i<n; i++) aa[i] = new double[m];
  ka = m + 1;
  if (m < n)  ka = n + 1;
  s = new double[ka];
  e = new double[ka];
  w = new double[ka];
  eps = 1e-7;
 }

 void input (int **temp_ATA);        
 void uav ();          
 void ppp();            
 void sss();           
 void a_ginv ();         
 void output (double *var_result, double *temp_ATC);        

~ginv() {
  int i;
  for (i=0; i<m; i++) 
  { 
   delete [] a[i]; 
  }
  delete [] a;

  for (i=0; i<m; i++) 
  { 
   delete [] u[i]; 
  }
  delete [] u;

  for (i=0; i<n; i++) 
  { 
   delete [] v[i]; 
  }
  delete [] v;
 
  for (i=0; i<n; i++) 
  { 
   delete [] aa[i]; 
  }
  delete [] aa;

  delete [] s, e, w;
}

 friend class DSRAgent;
};
//end persudo inverse class by CRJ


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

int **u_matrix;
int *temp_C, **temp_A, **temp_AT, **temp_ATA;
double  *temp_ATC, *var_result;

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
  struct Rec_Info rec_info[MAX_NEIGHBOR_NUMBER];
  struct Num_Rec_Info num_rec_info;
  struct Last_Rec_Info last_rec_info;
  struct Temp_Rvar temp_Rvar[MAX_NEIGHBOR_NUMBER];
  struct Temp_Cvar temp_Cvar[MAX_NEIGHBOR_NUMBER]; 
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
  friend class ginv;

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
//function related to compute recommend trust
  bool necessary_compute_bias();
  void release_memory(int row, int col);
  void compute_bias();
  void update_rec_info(Packet *p);
  void update_rec_info_time(); 
  void update_num_rec_info();
  void matrix_multiplication();
  void compute_recommended_trust(double *var_result);

//help debugging related functions
  void print_matrix(int **matrix, int row, int col);
  void collect_BWlist_info();
  void clear_BWlist_array();
  //void creat_temp_matrix(int row_size, int col_size);

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
