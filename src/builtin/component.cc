/* Copyright 2008 (C) Nicira, Inc.
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
#include "component.hh"

#include "event-dispatcher-component.hh"
#include "nox.hh"
#include "../oflib/ofl-messages.h"

using namespace std;
using namespace vigil;

namespace vigil {
namespace container {

// TODO: for now the class merely acts as a facade to controller
// namespace for event and timer functionality. change this.

Component::Component(const Context* ctxt_)
    : ctxt(ctxt_) {
}

Component::~Component() { 
    
}

    //Interface_description 
    //Component::getInterface() const {
    // return interface;
    //}

int 
Component::send_openflow_command(const datapathid& datapath_id, 
                                 const ofp_header* oh, bool block) const {
    return nox::send_openflow_command(datapath_id, oh, block);
}

int 
Component::send_openflow_command(const datapathid& datapath_id, 
                                 boost::shared_array<uint8_t>& of_raw, 
				 bool block) const {
    return nox::send_openflow_command(datapath_id, 
				      (ofp_header *) of_raw.get(), block);
}

int
Component::send_openflow_msg(const datapathid& dpid, struct ::ofl_msg_header *msg, uint32_t xid, bool block) const {
    return nox::send_openflow_msg(dpid, msg, xid, block);
}

int
Component::send_openflow_pkt(const datapathid& dpid, uint32_t buffer_id, uint32_t in_port, uint32_t out_port, bool block) {
    return nox::send_openflow_pkt(dpid, buffer_id, in_port, out_port, block);
}

int
Component::send_openflow_pkt(const datapathid& dpid, const Buffer& buffer, uint32_t in_port, uint32_t out_port, bool block) {
    return nox::send_openflow_pkt(dpid, buffer, in_port, out_port, block);
}

int 
Component::close_openflow_connection(const datapathid& dpid)
{
    return nox::close_openflow_connection(dpid);
}

void 
Component::register_event(const Event_name& name) const {
    EventDispatcherComponent* dispatcher;
    resolve<EventDispatcherComponent>(dispatcher);
    if (!dispatcher->register_event(name)) {
        throw runtime_error("Event '" + name +"' already exists.");   
    }
}

void
Component::post(Event* event) const {
    nox::post_event(event);
}

Timer
Component::post(const Timer_Callback& callback) const {
    return nox::post_timer(callback);
}

Timer
Component::post(const Timer_Callback& callback, const timeval& duration) const {
    return nox::post_timer(callback, duration);
}

void
Component::register_handler(const Event_name& event_name,
                            const Event_handler& h) const {
    EventDispatcherComponent* dispatcher;
    resolve<EventDispatcherComponent>(dispatcher);
    if (!dispatcher->register_handler(ctxt->get_name(), event_name, h)) {
        throw runtime_error("Event '" + event_name +"' doesn't exist.");
    }
}

Component::Rule_id
Component::register_handler_on_match(uint32_t priority, 
                                     const Packet_expr &expr, 
                                     Pexpr_action callback) const {
    return nox::register_handler_on_match(priority, expr, callback);
}

bool 
Component::unregister_handler(Rule_id rule_id) const {
    return nox::unregister_handler(rule_id);
}


uint32_t Component::get_switch_controller_ip(const datapathid &datapath_id){
    return nox::get_switch_controller_ip(datapath_id);
}

uint32_t Component::get_switch_ip(const datapathid &datapath_id){
    return nox::get_switch_ip(datapath_id);
}

void 
Component::register_switch_auth(Switch_Auth *auth) const { 
  nox::register_switch_auth(auth);   
} 

Context::~Context() { 
    
}

Component_factory::Component_factory() { 
}

Component_factory::~Component_factory() { 
}

Configuration::~Configuration() {
}

} // namespace container
} // namespace vigil
