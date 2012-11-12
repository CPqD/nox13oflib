#ifndef INSTRUCTIONS_HH
#define INSTRUCTIONS_HH 1

#include "actions.hh"
#include "../oflib/ofl-messages.h"
#include "../oflib/ofl-structs.h"

namespace vigil {

class Instruction {
public:
    struct ofl_instruction_header **insts;
    static int inst_num;

    Instruction();
    
    ~Instruction();

    void
    CreateApply(Actions *actions);
    
    void
    CreateGoToTable(uint8_t);
    
    void 
    CreateWrite(Actions *actions);
    
    void 
    CreateWriteMetadata(uint64_t metadata);
    void
    CreateWriteMetadata(uint64_t metadata, uint64_t metadata_mask);
    
    void 
    CreateClearActions();
    
    void
    CreateMeterInstruction(uint32_t meter_id);
    
    void 
    AddActions(struct ofl_instruction_actions *i, Actions *actions);
};

}//namespace vigil
#endif
