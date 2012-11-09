#include "flowmod.hh"
#include "vlog.hh"

namespace vigil {

static Vlog_module log("flowmod");

FlowMod::FlowMod() {
	init_flow_mod();
}

FlowMod::FlowMod(uint64_t cookie, uint64_t cookie_mask, uint8_t table_id,
                       enum ofp_flow_mod_command cmd,uint16_t idlet, uint16_t hardt, 
                       uint16_t prio, uint32_t buff_id, uint32_t out_port, 
                       uint32_t out_group, uint16_t flags){

    fm_msg.header.type = OFPT_FLOW_MOD;
    fm_msg.cookie = cookie;
    fm_msg.cookie_mask = cookie_mask;
    fm_msg.table_id = table_id;
    fm_msg.command = cmd;
    fm_msg.idle_timeout = idlet;
    fm_msg.hard_timeout = hardt;
    fm_msg.priority = prio;
    fm_msg.buffer_id = buff_id;
    fm_msg.out_port = out_port;
    fm_msg.out_group = out_group;
    fm_msg.flags = flags;
    fm_msg.match = NULL;
    fm_msg.instructions_num = 0;
    fm_msg.instructions = NULL;
}


void
FlowMod::init_flow_mod(){

    fm_msg.header.type = OFPT_FLOW_MOD;
    fm_msg.cookie = 0x0000000000000000ULL;
    fm_msg.cookie_mask = 0x0000000000000000ULL;
    fm_msg.table_id = 0xff;
    fm_msg.command = OFPFC_ADD;
    fm_msg.idle_timeout = OFP_FLOW_PERMANENT;
    fm_msg.hard_timeout = OFP_FLOW_PERMANENT;
    fm_msg.priority = OFP_DEFAULT_PRIORITY;
    fm_msg.buffer_id = 0xffffffff;
    fm_msg.out_port = OFPP_ANY;
    fm_msg.out_group = OFPG_ANY;
    fm_msg.flags = 0x0000;
    fm_msg.match = NULL;
    fm_msg.instructions_num = 0;
    fm_msg.instructions = NULL;
}


void
FlowMod::AddMatch(struct ofl_match *m){
    fm_msg.match = (struct ofl_match_header *)m;
}

void
FlowMod::set_instructions_num(int inst_num){

    fm_msg.instructions_num = inst_num;

}

void 
FlowMod::AddInstructions(Instruction *inst){
    
    set_instructions_num(inst->inst_num);
    fm_msg.instructions = inst->insts;
}


} // namespace vigil
