#include "instructions.hh"
#include "vlog.hh"
#include "iostream"
using namespace std;

namespace vigil {

static Vlog_module log("instructions");

int Instruction::inst_num;

Instruction::Instruction(){
    insts = (struct ofl_instruction_header **) xmalloc(sizeof(struct ofl_instruction_header*));
    inst_num = 0;     
}

Instruction::~Instruction(){

    for(int i = 0; i < inst_num; i++ ) 
        free(insts[i]);
}

void
Instruction::CreateApply(Actions *actions){
    
    inst_num++;
    insts = (struct ofl_instruction_header **) xrealloc(insts, sizeof(struct ofl_instruction_header**) * inst_num );
    struct ofl_instruction_actions *i = (struct ofl_instruction_actions*) xmalloc(sizeof(struct ofl_instruction_actions));
    i->header.type = OFPIT_APPLY_ACTIONS;
    i->actions = NULL;
    i->actions_num = 0;
    AddActions(i, actions);
    insts[inst_num-1] = (struct ofl_instruction_header *)i;
    
}
    
void
Instruction::CreateGoToTable(uint8_t table_id){

    inst_num++;
    insts = (struct ofl_instruction_header **) xrealloc(insts, sizeof(struct ofl_instruction_header**) * inst_num );
    struct ofl_instruction_goto_table *i = (struct ofl_instruction_goto_table*) xmalloc(sizeof(struct ofl_instruction_goto_table));
    i->header.type = OFPIT_GOTO_TABLE;
    i->table_id = table_id;
    insts[inst_num-1] = (struct ofl_instruction_header *)i;
    
}
    
void 
Instruction::CreateWrite(Actions *actions){
    
    inst_num++;
    insts = (struct ofl_instruction_header **) xrealloc(insts, sizeof(struct ofl_instruction_header**) * inst_num );
    struct ofl_instruction_actions *i = (struct ofl_instruction_actions*) xmalloc(sizeof(struct ofl_instruction_actions));
    i->header.type = OFPIT_WRITE_ACTIONS;
    i->actions = NULL;
    i->actions_num = 0;
    AddActions(i, actions);
    insts[inst_num-1] = (struct ofl_instruction_header *)i;
    
}
    
void 
Instruction::CreateWriteMetadata(uint64_t metadata){
    inst_num++;
    insts = (struct ofl_instruction_header **) xrealloc(insts, sizeof(struct ofl_instruction_header**) * inst_num );
    struct ofl_instruction_write_metadata *i = (struct ofl_instruction_write_metadata*) xmalloc(sizeof(struct ofl_instruction_write_metadata));
    i->header.type = OFPIT_WRITE_METADATA;  
    i->metadata = metadata;
    i->metadata_mask = 0xffffffffffffffffULL;
    insts[inst_num-1] = (struct ofl_instruction_header *)i;
}

void 
Instruction::CreateWriteMetadata(uint64_t metadata, uint64_t metadata_mask){
    inst_num++;
    insts = (struct ofl_instruction_header **) xrealloc(insts, sizeof(struct ofl_instruction_header**) * inst_num );
    struct ofl_instruction_write_metadata *i = (struct ofl_instruction_write_metadata*) xmalloc(sizeof(struct ofl_instruction_write_metadata));
    i->header.type = OFPIT_WRITE_METADATA;  
    i->metadata = metadata;
    i->metadata_mask = metadata_mask;
    insts[inst_num-1] = (struct ofl_instruction_header *)i;
    
}
 
void 
Instruction::CreateClearActions(){
    inst_num++;
    insts = (struct ofl_instruction_header **) xrealloc(insts, sizeof(struct ofl_instruction_header**) * inst_num );
    struct ofl_instruction_header *i = (struct ofl_instruction_header*) xmalloc(sizeof(struct ofl_instruction_header));
    i->type = OFPIT_CLEAR_ACTIONS;
    insts[inst_num-1] = (struct ofl_instruction_header *)i;
}

void
Instruction::CreateMeterInstruction(uint32_t meter_id){

    inst_num++;
    insts = (struct ofl_instruction_header **) xrealloc(insts, sizeof(struct ofl_instruction_header**) * inst_num );
    struct ofl_instruction_meter *i = (struct ofl_instruction_meter*) xmalloc(sizeof(struct ofl_instruction_meter));
    i->header.type = OFPIT_METER;
    i->meter_id = meter_id;
    insts[inst_num-1] = (struct ofl_instruction_header *)i;

}

void 
Instruction::AddActions(struct ofl_instruction_actions *i, Actions *actions){

    i->actions = actions->acts;
    i->actions_num = actions->act_num;

}

} // namespace vigil
