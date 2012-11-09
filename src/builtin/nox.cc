/* Copyright 2008, 2009 (C) Nicira, Inc.
 *
 * This file is part of NOX.
 *
 * NOX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NOX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NOX.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nox.hh"

#include <boost/bind.hpp>
#include <boost/shared_array.hpp>
#include <errno.h>
#include <map>
#include <signal.h>
#include <inttypes.h>
#include "kernel.hh" 
#include "assert.hh"
#include "buffer.hh"
#include "datapath-join.hh"
#include "datapath-leave.hh"
#include "event-dispatcher.hh"
#include "ofp-msg-event.hh"
#include "openflow.hh"
#include "openflow/nicira-ext.h"
#include "poll-loop.hh"
#include "shutdown-event.hh"
#include "string.hh"
#include "threads/signals.hh"
#include "timeval.hh"
#include "vlog.hh"
#include "switch_auth.hh"

#include "../oflib/ofl.h"
#include "../oflib/ofl-actions.h"
#include "../oflib/ofl-structs.h"
#include "../oflib/ofl-messages.h"

namespace vigil {
namespace nox {

static Vlog_module lg("nox");

static Packet_classifier classifier;

static const int N_THREADS = 8;
static Poll_loop* main_loop;
static Event_dispatcher event_dispatcher;
static Timer_dispatcher timer_dispatcher;
static Switch_Auth *switch_authenticator = NULL; 

class Conn
    : public Pollable {
public:
    boost::shared_ptr<Openflow_connection> oconn;
    Co_sema* disconnected;

    Conn(boost::shared_ptr<Openflow_connection> oconn_,
         Co_sema* disconnected_);
    ~Conn();

    bool poll();
    void wait();
    
    void close();
private:
    bool closing;
    int poll_cnt;

    bool do_poll();
};

// DPID to connection mappings 
typedef std::map<datapathid, Conn*> chashmap;
static chashmap connection_map;

Conn::Conn(boost::shared_ptr<Openflow_connection> oconn_,
           Co_sema* disconnected_)
    : oconn(oconn_),
      disconnected(disconnected_),
      closing(false),
      poll_cnt(0)
{
    main_loop->add_pollable(this);
}

Conn::~Conn()
{
    close();
}

static boost::shared_ptr<Openflow_connection>
dpid_to_oconn(datapathid dpid)
{
    chashmap::iterator iter = connection_map.find(dpid);
    if (iter == connection_map.end()) {
        lg.err("no datapath with id %s registered at nox",
               dpid.string().c_str());
        return boost::shared_ptr<Openflow_connection>();
    }
    return iter->second->oconn;
}


bool
Conn::poll()
{
    ++poll_cnt;
    bool retval = do_poll();
    int p = --poll_cnt;
    if (!p && closing) {
        delete this;
    }
    return retval;
}

void
Conn::close()
{
    if (!closing) {
        closing = true;
        datapathid dp_id = oconn->get_datapath_id();
        Datapath_leave_event* dple = new Datapath_leave_event(dp_id);
        post_event(dple);

        connection_map.erase(dp_id);
        main_loop->remove_pollable(this);
        if (!poll_cnt) {
            delete this;
        }
    }
}

bool
Conn::do_poll()
{
    int error;
    std::auto_ptr<Buffer> b(oconn->recv_openflow(error, false));
    switch (error) {
    case 0: {

        struct ofl_msg_header *ofl_msg;
        uint32_t xid;
        if (ofl_msg_unpack(b.get()->data(), b.get()->size(), &ofl_msg, &xid, NULL/*ofl_exp*/)) {
            lg.warn("Error unpacking OpenFlow message.");
            return false;
        }
        Ofp_msg *ofp_msg = new Ofp_msg(ofl_msg);
        std::auto_ptr<Event> event(Ofp_msg_event::create_event(oconn->get_datapath_id(), xid, boost::shared_ptr<Ofp_msg>(ofp_msg)));

        if (event.get() != NULL) {
            event_dispatcher.dispatch(*event);
        }

        return true;
    }

    case EAGAIN:
        return false;

    case EOF:
        lg.warn("%s: connection closed by peer", oconn->to_string().c_str());
        close();
        return true;
 
    default:
        lg.warn("%s: disconnected (%s)",
                oconn->to_string().c_str(), strerror(error));
        close();
        return true;
   }
}

void
Conn::wait()
{
    oconn->recv_openflow_wait();
}

class Signal_handler {
public:
    Signal_handler();
    ~Signal_handler();
private:
    Co_fsm fsm;
    Signal_group* sig_group;
    bool shutdown_in_progress;

    void run();
    static Disposition exit(const Event&) { ::exit(0); }
};

Signal_handler::Signal_handler()
    : sig_group(new Signal_group),
      shutdown_in_progress(false)
{
    sig_group->add_signal(SIGTERM);
    sig_group->add_signal(SIGINT);
    sig_group->add_signal(SIGHUP);
    fsm.start(boost::bind(&Signal_handler::run, this));
    register_handler(Shutdown_event::static_get_name(), exit, 9999);
}

Signal_handler::~Signal_handler()
{
    sig_group->remove_all();
}

void
Signal_handler::run()
{
    if (sig_group->try_dequeue() && !shutdown_in_progress) {
        shutdown_in_progress = true;
        post_event(new Shutdown_event);
    }
    if (!shutdown_in_progress) {
        sig_group->wait();
    }
    co_fsm_block();
}

static Disposition
handle_echo_request(const Event& e)
{
    const Ofp_msg_event& echo = assert_cast<const Ofp_msg_event&>(e);
    if (boost::shared_ptr<Openflow_connection> oconn
        = dpid_to_oconn(echo.dpid)) {
        oconn->send_echo_reply(echo.xid, echo.msg);
    }

    return CONTINUE;
}

void
init()
{
    classifier.register_packet_in();
    register_handler("Echo_request_event",
                     handle_echo_request, 100);
    main_loop = new Poll_loop(N_THREADS);
    main_loop->add_pollable(&event_dispatcher);
    main_loop->add_pollable(&timer_dispatcher);
    new Signal_handler;
}

Poll_loop*
get_poll_loop() {
    return main_loop;
}

void
register_handler(const Event_name& name,
                 boost::function<Disposition(const Event&)> handler, int order)
{
    event_dispatcher.add_handler(name, handler, order);
}

/* Returns a nonzero OpenFlow transaction ID that has not been used for some
 * time.  Transaction IDs are per-datapath (actually, per connection to a given
 * datapath), so this is more uniqueness than needed, but the available space
 * is large enough that there is no disadvantage to it.  */
uint32_t
allocate_openflow_xid()
{
    static uint32_t xid;
    if (!xid) {
        xid = 1;
    }
    return xid++;
}

/* Attempts to send OpenFlow command 'oh' to switch 'datapath_id'.
 *
 * Returns 0 if successful or a positive errno value.  Returns ESRCH if switch
 * 'datapath_id' is unknown.  If 'block' is true, blocks as necessary;
 * otherwise, returns EAGAIN if blocking is needed.  */
int send_openflow_command(const datapathid& datapath_id, const ofp_header* oh,
                          bool block)
{
    co_might_yield_if(block);
    boost::shared_ptr<Openflow_connection> oconn = dpid_to_oconn(datapath_id);
    if (!oconn) {
        return ESRCH;
    }

    int error = oconn->send_openflow(oh, block);
    //NOTE: Originally a flowmod event was generated here when a flowmod was
    //      sent to the switch
    return error;
}

int
send_openflow_msg(const datapathid& dpid, struct ::ofl_msg_header *msg, uint32_t xid, bool block) {
    uint8_t *buf;
    size_t buf_len;

    int error = ofl_msg_pack(msg, xid, &buf, &buf_len, NULL/*ofl_exp*/);
    if (error) {
        return error;
    }
    int ret = send_openflow_command(dpid, (const ofp_header*)buf, block);
    free(buf);
    return ret;
}

int
send_openflow_pkt(const datapathid& dpid, uint32_t buffer_id, uint32_t in_port, uint32_t out_port, bool block) {
    struct ofl_action_output output =
            {{/*.type = */OFPAT_OUTPUT}, /*.port = */out_port, /*.max_len = */0};

    struct ofl_action_header *actions[] =
            { (struct ofl_action_header *)&output };

    struct ofl_msg_packet_out out =
            {{/*.type       = */OFPT_PACKET_OUT},
             /*.buffer_id   = */buffer_id,
             /*.in_port     = */in_port,
             /*.actions_num = */1,
             /*.actions     = */actions,
             /*.data_length = */0,
             /*.data        = */NULL};

    return send_openflow_msg(dpid, (struct ofl_msg_header *)&out, 0/*xid*/, block);
}

int
send_openflow_pkt(const datapathid& dpid, const Buffer& buffer, uint32_t in_port, uint32_t out_port, bool block) {
    struct ofl_action_output output =
            {{/*.type = */OFPAT_OUTPUT}, /*.port = */out_port, /*.max_len = */0};

    struct ofl_action_header *actions[] =
            { (struct ofl_action_header *)&output };

    struct ofl_msg_packet_out out =
            {{/*.type       = */OFPT_PACKET_OUT},
             /*.buffer_id   = */0xffffffff, // no buffer
             /*.in_port     = */in_port,
             /*.actions_num = */1,
             /*.actions     = */actions,
             /*.data_length = */buffer.size(),
             /*.data        = */(uint8_t *)buffer.data()};

    return send_openflow_msg(dpid, (struct ofl_msg_header *)&out, 0/*xid*/, block);
}


int close_openflow_connection(const datapathid& dpid)
{
    chashmap::iterator iter = connection_map.find(dpid);
    if (iter == connection_map.end()) {
        lg.err("request to close connection to unknown dpid '%s'\n",
               dpid.string().c_str());
        return ESRCH;
    }
    iter->second->close();
    return 0; // success
}



uint32_t get_switch_controller_ip(const datapathid &datapath_id){
    boost::shared_ptr<Openflow_connection> oconn = dpid_to_oconn(datapath_id);
    if (!oconn) {
        return 0; //indicates invalid IP 0.0.0.0
    }
    return oconn->get_local_ip();
}

uint32_t get_switch_ip(const datapathid &datapath_id){
    boost::shared_ptr<Openflow_connection> oconn = dpid_to_oconn(datapath_id);
    if (!oconn) {
        return 0; //indicates invalid IP 0.0.0.0
    }
    return oconn->get_remote_ip();
}

class Log_fetcher {
public:
    Log_fetcher(boost::shared_ptr<Tcp_socket>& tcp,
                const std::string& output_name_,
                const boost::function<void(int error,
                                           const std::string& msg)>& cb_)
        : listener(tcp),
          cb(cb_),
          output_name(output_name_),
          output(NULL),
          fsm(boost::bind(&Log_fetcher::run_listen, this)) { }
    ~Log_fetcher() {
        if (output) {
            fclose(output);
        }
    }
private:
    boost::shared_ptr<Tcp_socket> listener;
    boost::shared_ptr<Tcp_socket> receiver;
    boost::function<void(int error, const std::string& msg)> cb;
    const std::string& output_name;
    FILE *output;
    Auto_fsm fsm;

    void run_listen();
    void run_receive();
    void die(int error, const std::string& msg);
};

void
Log_fetcher::run_listen()
{
    int error;
    std::auto_ptr<Tcp_socket> sock(listener->accept(error, false));
    if (error == EAGAIN) {
        sock->accept_wait();
        fsm.block();
    } else if (error) {
        die(error, "accept failed");
    } else {
        listener->close();
        output = fopen(output_name.c_str(), "w");
        if (output) {
            receiver = boost::shared_ptr<Tcp_socket>(sock.release());
            fsm.transition(boost::bind(&Log_fetcher::run_receive, this));
        } else {
            die(errno, string_format("could not create output file %s: %s",
                                     output_name.c_str(), strerror(errno)));
        }
    }
}

void
Log_fetcher::run_receive()
{
    Array_buffer b(4096);
    int error = receiver->read(b, false);
    if (error == -EAGAIN || error > 0) {
        if (error > 0) {
            fwrite(b.data(), 1, b.size(), output);
        }
        receiver->read_wait();
        fsm.block();
    } else if (!error) {
        die(0, "success");
    } else {
        die(-error, string_format("error reading socket: %s",
                                  strerror(error)));
    }
}

void
Log_fetcher::die(int error, const std::string& msg)
{
    lg.warn("log file retrieval complete: %s", msg.c_str());

    /* Close the file before invoking the callback, to ensure that it is
     * flushed to disk.  */
    if (output) {
        fclose(output);
        output = NULL;
    }

    cb(error, msg);
    fsm.exit();
    delete this;
}

void
post_event(Event* event)
{
    event_dispatcher.post(event);
}

Timer
post_timer(const Callback& callback, const timeval& duration)
{
    return timer_dispatcher.post(callback, duration);
}

Timer
post_timer(const Callback& callback)
{
    return timer_dispatcher.post(callback);
}

void
timer_debug() {
    timer_dispatcher.debug();
}

//-----------------------------------------------------------------------------

uint32_t 
register_handler_on_match(uint32_t priority, const Packet_expr &expr,
                          Pexpr_action callback)
{
    return classifier.add_rule(priority, expr, callback);
}

bool 
unregister_handler(uint32_t rule_id)
{
    return classifier.delete_rule(rule_id);
}

void register_switch_auth(Switch_Auth* auth) { 
  if(switch_authenticator) { 
    lg.err("Switch Auth already set, ignoring register_switch_auth\n");
    return; 
  } 
  switch_authenticator = auth; 
} 

Switch_Auth* get_switch_auth() { 
  return switch_authenticator; 
} 

/* FSM responsible for Openflow_connections on which we have not yet received 
 * a features reply message.  It transmits (and retransmits, if necessary) 
 * features request messages until it receives a reply, at which point it 
 * registers the connection with nox under its datapath id and kills
 * itself.
 *
 * Kind of a mess, really.  Needs refactoring. */
class Handshake_fsm {
public:
    Handshake_fsm(std::auto_ptr<Openflow_connection>, 
              Co_sema* disconnected, int *errorp, int timeout_secs_);
private:
    enum Handshake_state { SEND_FEATURES_REQ = 0, 
                           SEND_CONFIG,
                           RECV_FEATURES_REPLY,
                           CHECK_SWITCH_AUTH, 
                           REGISTER_SWITCH, 
                           NUM_STATES };

    static std::string state_desc[NUM_STATES]; 
    std::auto_ptr<Openflow_connection> oconn;
    int timeout_secs;
    timeval timeout;            /* Time to give up on the connection. */
    Co_sema* disconnected;
    int *errorp;
    Handshake_state state;
    Co_completion completion; // used to signal switch approval is done
    bool switch_approved;

    // Buffer to handle OFMP extended data messages
    std::auto_ptr<Buffer> ext_data_buffer; 
    uint32_t ext_data_xid;

    /* We need to buffer features reply so that we can create 
     * a Datapath_join_event if asynchronous switch authorization succeeds */ 

    boost::shared_ptr<Ofp_msg> features_reply;

    void run();
    void send_message();
    void recv_message();
    void check_switch_auth(Handshake_state next); 
    void switch_auth_cb(bool is_valid, Handshake_state next); 
    void register_switch(); 
    void do_exit(int error);
};

std::string Handshake_fsm::state_desc[NUM_STATES] = { 
                                          "sending features request", 
                                          "sending switch config", 
                                          "receiving features reply",
                                          "checking switch auth",
                                          "registering switch"};

Handshake_fsm::Handshake_fsm(std::auto_ptr<Openflow_connection> oconn_,
                    Co_sema* disconnected_, int* errorp_, int timeout_secs_)
    : oconn(oconn_),
      timeout_secs(timeout_secs_),
      timeout(do_gettimeofday() + make_timeval(timeout_secs_,0)),
      disconnected(disconnected_),
      errorp(errorp_),
      state(SEND_FEATURES_REQ),
      switch_approved(false), 
      ext_data_xid(UINT32_MAX),
      features_reply()
{
    ext_data_buffer.reset(new Array_buffer(0));
    co_fsm_create(co_group_self(), boost::bind(&Handshake_fsm::run, this));
}

void Handshake_fsm::send_message() {
    int error; 
    Handshake_state next_state; 
    if(state == SEND_FEATURES_REQ) {
      error = oconn->send_features_request();
      next_state = SEND_CONFIG; 
    } else if (state == SEND_CONFIG) { 
      error = oconn->send_switch_config(); 
      next_state = RECV_FEATURES_REPLY; 
    } else { 
      lg.err("Invalid state %d in Handshake_fsm::send_openflow()\n", state);
      do_exit(EINVAL);
      return; 
    } 
    
    if (error == EAGAIN) {
        oconn->send_openflow_wait();
        co_timer_wait(timeout, NULL);
        co_fsm_block();
    } else if (error) {
        lg.warn("Error %s: %s", state_desc[state].c_str(), strerror(error));
        do_exit(error);
    } else {
        state = next_state; 
        lg.dbg("Success sending in '%s'", state_desc[state].c_str());
        co_fsm_yield();
    }
}


void Handshake_fsm::recv_message() {

    int error;
    std::auto_ptr<Buffer> buf = oconn->recv_openflow(error, false);
    if (error == EAGAIN) {
        oconn->recv_openflow_wait();
        co_timer_wait(timeout, NULL);
        co_fsm_block();
        return; 
    } else if (error) {
        lg.warn("Error %s : recv: %s", state_desc[state].c_str(),
                error == EOF ? "connection closed" : strerror(error));
        do_exit(error);
        return; 
    }

    struct ofl_msg_header *ofl_msg;
    uint32_t xid;
    if (ofl_msg_unpack(buf->data(), buf->size(), &ofl_msg, &xid, NULL/*ofl_exp*/)) {
        //TODO: Log error
    } else {
      lg.dbg("Success receiving in '%s'", state_desc[state].c_str());

      boost::shared_ptr<Ofp_msg> msg(new Ofp_msg(ofl_msg));

      switch ((*msg)->type) {
        case OFPT_FEATURES_REPLY:
          if(state != RECV_FEATURES_REPLY) { 
              lg.warn("Ignoring features reply "
                      "received while in state '%s'", 
                      state_desc[state].c_str());
              break; 
          }

          // save features reply for switch auth and registration 
          features_reply = msg;
          state = CHECK_SWITCH_AUTH;
          break;
        case OFPT_ECHO_REQUEST:
          oconn->send_echo_reply(xid, msg);
          break;
        case OFPT_ERROR: {
            struct ofl_msg_error *error = (struct ofl_msg_error *)**msg;
            lg.warn("Received error during handshake (%u/%u)",
                    error->type, error->code);
            do_exit(EINVAL);
            return;
        }
        case OFPT_PACKET_IN:
          /* These messages can be received before nox considers the
           * handshake to be complete, and don't indicate an error. */
          lg.dbg("Dropping packet in message during handshake");
          break;
        default:
          lg.warn("Received unsupported message type during handshake (%d)",
                  (*msg)->type);
          break;
      }
    }
    co_fsm_yield();
}

void Handshake_fsm::check_switch_auth(Handshake_state next) {
 
  Switch_Auth* auth = nox::get_switch_auth(); 
  if(!auth) { 
    lg.dbg("No switch auth module registered, auto-approving switch\n"); 
    switch_approved = true;
    state = next; 
    run();
    return; 
  }  
  assert(features_reply);
  auth->check_switch_auth(oconn, features_reply, 
      boost::bind(&Handshake_fsm::switch_auth_cb,this,_1,next));
  completion.wait(NULL); 
  co_fsm_block();
} 

void Handshake_fsm::switch_auth_cb(bool is_approved,Handshake_state next) {
  switch_approved = is_approved;
  state = next; 
  completion.release(); // restart FSM
} 



void Handshake_fsm::register_switch() { 
    datapathid dpid = datapathid::from_host(((struct ofl_msg_features_reply*)**features_reply)->datapath_id);
    if(!switch_approved) { 
      lg.err("Disconnecting unapproved switch %"PRIx64" \n", dpid.as_host());
      do_exit(EPERM);
      return; 
    }
    
    if(dpid.as_host() == 0) { 
      lg.err("0 is not a valid DPID. Disconnecting switch");
      do_exit(EINVAL);
      return; 
    }

    oconn->set_datapath_id(dpid);
    register_conn(oconn.release(), disconnected);

    /* delete all flows on this switch 
    struct ofl_match_standard match;
    match.header.type = OFPMT_STANDARD;
    match.wildcards = OFPFW_ALL;
    memset(match.dl_src_mask, 0xff, 6);
    memset(match.dl_dst_mask, 0xff, 6);
    match.nw_src_mask = 0xffffffff;
    match.nw_dst_mask = 0xffffffff;
    match.metadata_mask = 0xffffffffffffffffULL;

    struct ofl_msg_flow_mod mod;
    mod.header.type = OFPT_FLOW_MOD;
    mod.cookie = 0x00ULL;
    mod.cookie_mask = 0x00ULL;
    mod.table_id = 0xff; // all tables
    mod.command = OFPFC_DELETE;
    mod.out_port = OFPP_ANY;
    mod.out_group = OFPG_ANY;
    mod.flags = 0x0000;
    mod.match = (struct ofl_match_header *)&match;
    mod.instructions_num = 0;
    mod.instructions = NULL;

    /* XXX OK to do non-blocking send?  We do so with all other
     * commands on switch join 
    if ( send_openflow_msg(dpid, (struct ofl_msg_header *)&mod, 0/*xid, false) == EAGAIN) {
          lg.err("Error, unable to clear flow table on startup");
    }
    */
    lg.dbg("Registering switch with DPID = %"PRIx64"\n",dpid.as_host()); 
    /* Really we want to just dispatch this event immediately, but that would
     * prevent any handlers for it from blocking, since we're running inside
     * an FSM. */
    // NOTE: event steals auto_ptr to features
    event_dispatcher.post(new Datapath_join_event(dpid, 0/*xid*/, features_reply));
    disconnected = NULL;

    do_exit(0);
}


void Handshake_fsm::run()
{
    timeval now = do_gettimeofday();
    if (now > timeout) {
        lg.warn("%s: closing connection due to timeout after %d seconds in "
                "%s state ", oconn->to_string().c_str(), timeout_secs,
                state_desc[state].c_str());
        do_exit(ETIMEDOUT);
        return;
    }

    switch (state) { 
      case SEND_FEATURES_REQ: 
        send_message();
        break;  
      case SEND_CONFIG: 
        send_message();
        break;  
      case RECV_FEATURES_REPLY: 
        recv_message(); 
        break; 
      case CHECK_SWITCH_AUTH: 
        check_switch_auth(REGISTER_SWITCH);
        break;
      case REGISTER_SWITCH: 
        register_switch(); 
        break; 
      default: 
        lg.err("Invalid Handshake state = %d \n", state); 
    } 
}

void Handshake_fsm::do_exit(int error) 
{
    if (errorp) {
        *errorp = error;
    }
    if (disconnected) {
        disconnected->up();
    }
    co_fsm_exit();
    delete this; 
}

/* Connector threads.
 *
 * This code is responsible for taking a Openflow_connection_factory and
 * connecting to it, then creating a Handshake_fsm for it.
 *
 * There's no reason that this code has to run as a thread.  It used to have
 * to because some Openflow_connections had connect() functions that blocked.
 * They don't anymore, so eventually the connector threads should probably
 * become FSMs to save the cost of a thread. */

struct Connector_aux {
    Openflow_connection_factory* factory;
};

int do_connect(Connector_aux* aux, int timeout_sec,
               Co_sema *disconnected, int *errorp)
{
    for (;;) {
        int error;
        std::auto_ptr<Openflow_connection> oconn(aux->factory->connect(error));
        if (error != EAGAIN) {
            if (!error) {
                new Handshake_fsm(oconn, disconnected, errorp, timeout_sec);
            }
            return error;
        }
        aux->factory->connect_wait();
        co_block();
    }
}

void unreliable_connector_thread(Connector_aux* aux) 
{
    do_connect(aux, 60, NULL, NULL);
}

void passive_connector_thread(Connector_aux* aux)
{
    for (;;) {
        do_connect(aux, 5, NULL, NULL);
    }
}

void
connect(Openflow_connection_factory* factory, bool reliable)
{
    Connector_aux* aux = new Connector_aux;
    aux->factory = factory;

    if (reliable && !factory->passive()) {
        std::auto_ptr<Openflow_connection_factory> f(factory);
        std::auto_ptr<Openflow_connection> c(
            new Reliable_openflow_connection(f));
        new Handshake_fsm(c, NULL, NULL, 4);
    } else {
        void (*thread_func)(Connector_aux*);
        if (factory->passive()) {
            thread_func = passive_connector_thread; 
        } else {
            thread_func = unreliable_connector_thread;
        }
        co_thread_create(&co_group_coop, boost::bind(thread_func, aux)); 
    }
}

void
register_conn(Openflow_connection* oconn, Co_sema* disconnected) 
{
    datapathid dp_id = oconn->get_datapath_id();

    chashmap::iterator i;
    Conn* conn = new Conn(boost::shared_ptr<Openflow_connection>(oconn),
                          disconnected);
    std::pair<chashmap::iterator, bool> pair
        = connection_map.insert(chashmap::value_type(dp_id, conn));
    if (!pair.second) {
        pair.first->second->close();
        // Conn->close() removes the entry from connection_map, so reinsert
        pair = connection_map.insert(chashmap::value_type(dp_id, conn));
        assert(pair.second); 
    }
}

void run()
{
    main_loop->run();
}

} // namespace vigil::nox
} // namespace vigil
