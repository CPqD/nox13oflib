#ifndef FLOWMOD_HH
#define FLOWMOD_HH 1

#include "flow.hh"
#include "../oflib/ofl-messages.h"
#include "instructions.hh"

namespace vigil {

class FlowMod{
public:
    struct ofl_msg_flow_mod fm_msg;
 
    /** Empty constructor
    */
    FlowMod();
    
    FlowMod(uint64_t cookie, uint64_t cookie_mask, uint8_t table_id,
                   enum ofp_flow_mod_command cmd,uint16_t idlet, uint16_t hardt, 
                   uint16_t prio, uint32_t buff_id, uint32_t out_port, 
                   uint32_t out_group, uint16_t flags);
    void
    init_flow_mod();
    
   
    void
    AddMatch(struct ofl_match *m);
    
    void
    set_instructions_num(int inst_num);
    
    void 
    AddInstructions(Instruction *inst);
};

}//namespace vigil

#endif


