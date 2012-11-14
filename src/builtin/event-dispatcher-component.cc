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
#include "event-dispatcher-component.hh"

#include "datapath-join.hh"
#include "datapath-leave.hh"
#include "bootstrap-complete.hh"
#include "shutdown-event.hh"
#include "ofp-msg-event.hh"

#include "nox.hh"
#include "vlog.hh"
#include "json_object.hh"
#include "json-util.hh"

#include <iostream>

using namespace std;
using namespace vigil;
using namespace vigil::container;

static Vlog_module lg("event-dispatcher-c");

EventDispatcherComponent::EventDispatcherComponent(const Context* c,
                                                   const json_object*
                                                   platformconf)  
                                                   
    : Component(c) {
    // Construct the 'filter sequences' defined the JSON configuration file  
    json_object* nox = json::get_dict_value(platformconf, "nox");    
    json_object* events = json::get_dict_value(nox, "events");
    json_dict* eventsDict = (json_dict*) events->object;
    
    // For every event defined in the configuration
    json_dict::iterator event_di;
    for(event_di=eventsDict->begin(); event_di!=eventsDict->end(); ++event_di) {
        string event_name = event_di->first;
        int order = 0;
        json_array::iterator li;
        json_array* compList = (json_array*) event_di->second->object;
        // for every filter under the event
	EventFilterChain chain;
        for(li=compList->begin(); li!=compList->end(); ++li) {
            // add the filter in the event's filter_chain
            string filter = (((json_object*)*li)->get_string(true));
            chain[filter] = order++;
        }
	filter_chains[event_name] = chain;
    }

    // Register the system events
    register_event<Datapath_join_event>();
    register_event<Datapath_leave_event>();
    register_event<Shutdown_event>();
    register_event<Bootstrap_complete_event>();

    register_event(Ofp_msg_event::get_name(OFPT_HELLO));
    register_event(Ofp_msg_event::get_name(OFPT_ERROR));
    register_event(Ofp_msg_event::get_name(OFPT_ECHO_REQUEST));
    register_event(Ofp_msg_event::get_name(OFPT_ECHO_REPLY));
    register_event(Ofp_msg_event::get_name(OFPT_EXPERIMENTER));
    register_event(Ofp_msg_event::get_name(OFPT_FEATURES_REPLY));
    register_event(Ofp_msg_event::get_name(OFPT_GET_CONFIG_REPLY));
    register_event(Ofp_msg_event::get_name(OFPT_PACKET_IN));
    register_event(Ofp_msg_event::get_name(OFPT_FLOW_REMOVED));
    register_event(Ofp_msg_event::get_name(OFPT_PORT_STATUS));
    register_event(Ofp_msg_event::get_stats_name(OFPMP_DESC));
    register_event(Ofp_msg_event::get_stats_name(OFPMP_FLOW));
    register_event(Ofp_msg_event::get_stats_name(OFPMP_AGGREGATE));
    register_event(Ofp_msg_event::get_stats_name(OFPMP_TABLE));
    register_event(Ofp_msg_event::get_stats_name(OFPMP_PORT_STATS));
    register_event(Ofp_msg_event::get_stats_name(OFPMP_QUEUE));
    register_event(Ofp_msg_event::get_stats_name(OFPMP_GROUP));
    register_event(Ofp_msg_event::get_stats_name(OFPMP_GROUP_DESC));
    register_event(Ofp_msg_event::get_stats_name(OFPMP_GROUP_FEATURES));     
    register_event(Ofp_msg_event::get_stats_name(OFPMP_METER)); 
    register_event(Ofp_msg_event::get_stats_name(OFPMP_METER_CONFIG));   
    register_event(Ofp_msg_event::get_stats_name(OFPMP_TABLE_FEATURES));   
    register_event(Ofp_msg_event::get_stats_name(OFPMP_PORT_DESC));         
    register_event(Ofp_msg_event::get_name(OFPT_BARRIER_REPLY));
    register_event(Ofp_msg_event::get_name(OFPT_QUEUE_GET_CONFIG_REPLY));

}

Component*
EventDispatcherComponent::instantiate(const Context* ctxt, 
                                      const json_object* platform_conf) {
    return new EventDispatcherComponent(ctxt, platform_conf);
}

void 
EventDispatcherComponent::getInstance(const Context* ctxt, 
                                      EventDispatcherComponent*& edc) {
    edc = dynamic_cast<EventDispatcherComponent*>
        (ctxt->get_by_interface(container::Interface_description
                                (typeid(EventDispatcherComponent).name())));
}

void
EventDispatcherComponent::configure(const container::Configuration*) {
    
}

void
EventDispatcherComponent::install() {

}

void
EventDispatcherComponent::post(Event* event) const {
    // Not needed yet, component class uses still nox::post
    // directly. TBD.
}

bool
EventDispatcherComponent::register_event(const Event_name& name) const {
    if (events.find(name) != events.end()) {
        return false;
    } else {
        events.insert(name);
        return true;
    }
}

bool
EventDispatcherComponent::register_handler(const Component_name& filter,
                                           const Event_name& name,
                                           const Event_handler& h) const {
    if (events.find(name) == events.end()) {
        return false;
    }

    if (filter_chains.find(name) == filter_chains.end()) {
        nox::register_handler(name, h, 0);
    } else {
        EventFilterChain& chain = filter_chains[name];
        if (chain.find(filter) == chain.end()) {
            nox::register_handler(name, h, 0);
        } else {
            nox::register_handler(name, h, chain[filter]);
        }
    }

    return true;
}

/*EventDispatcherComponentFactory::EventDispatcherComponentFactory(const xercesc::DOMNode* conf_)
    : conf(conf_) { }

container::Component* 
EventDispatcherComponentFactory::instance(const container::Context* c, 
                                          const xercesc::DOMNode* xml) const { 
    return new EventDispatcherComponent(c, 
                                        //string(typeid(EventDispatcherComponent).name()), 
                                        //name, 
                                        conf);
}

void
EventDispatcherComponentFactory::destroy(container::Component*) const { 
ctxt->install(from,() */
