#include "actions.hh"
#include "vlog.hh"

namespace vigil {

static Vlog_module log("actions");

int Actions::act_num;

Actions::Actions(){
    act_num = 0;
    acts = (struct ofl_action_header**) xmalloc(sizeof(struct ofl_action_header *));
}

void
Actions::CreateOutput(uint32_t port, uint16_t max_len){
    
    if (act_num)
        acts = (struct ofl_action_header**) xrealloc(acts, sizeof(struct ofl_action_header *) * (act_num +1)); 
    struct ofl_action_output *a = (struct ofl_action_output*) xmalloc(sizeof(struct ofl_action_output));
    a->port = port;
    a->max_len = max_len;
    acts[act_num] = (struct ofl_action_header*) a;
    acts[act_num]->type = OFPAT_OUTPUT;
    act_num++;
}
   
void
Actions::CreateOutput(uint32_t port){
    
    if (act_num)
        acts = (struct ofl_action_header**) xrealloc(acts, sizeof(struct ofl_action_header *) * (act_num +1)); 
    struct ofl_action_output *a = (struct ofl_action_output*) xmalloc(sizeof(struct ofl_action_output));
    a->port = port;
    a->max_len = 0;
    acts[act_num] = (struct ofl_action_header*) a;
    acts[act_num]->type = OFPAT_OUTPUT;
    act_num++;
} 

void 
Actions::CreateCopyTTL(enum ofp_action_type type){

    if (type != OFPAT_COPY_TTL_IN && type != OFPAT_COPY_TTL_OUT ){
        //#TODO Error message
    }
    else {
        if (act_num)
            acts = (struct ofl_action_header**) xrealloc(acts, sizeof(struct ofl_action_header *) * (act_num +1)); 
        acts[act_num]->type = type;
        struct ofl_action_header *a = (struct ofl_action_header*) xmalloc(sizeof(struct ofl_action_header));
        acts[act_num] = (struct ofl_action_header*) a;
        act_num++;
    }
}
    
void 
Actions::CreateDecTTL(enum ofp_action_type type){
    
    if (type != OFPAT_DEC_MPLS_TTL && type != OFPAT_DEC_NW_TTL ){
        //#TODO Error message
    }
    else {
        if (act_num)
            acts = (struct ofl_action_header**) xrealloc(acts, sizeof(struct ofl_action_header *) * (act_num +1)); 
        struct ofl_action_header *a = (struct ofl_action_header*) xmalloc(sizeof(struct ofl_action_header));
        acts[act_num] = (struct ofl_action_header*) a;
        acts[act_num]->type = type;
        act_num++;
    }
}
    
void 
Actions::CreatePushAction(enum ofp_action_type type, uint16_t ethertype){

    if (type != OFPAT_PUSH_MPLS && type != OFPAT_PUSH_VLAN && type != OFPAT_PUSH_PBB){
        //#TODO Error message
    }
    else {
        if (act_num)
            acts = (struct ofl_action_header**) xrealloc(acts, sizeof(struct ofl_action_header *) * (act_num +1));    
        struct ofl_action_push *psh = (struct ofl_action_push*) xmalloc(sizeof(struct ofl_action_push));
        psh->ethertype = ethertype;
        acts[act_num] = (struct ofl_action_header*) psh;
        acts[act_num]->type = type;
        act_num++;
    }
}
    
void 
Actions::CreatePopVlan(){
    
    if (act_num)
        acts = (struct ofl_action_header**) xrealloc(acts, sizeof(struct ofl_action_header *) * (act_num +1)); 
    struct ofl_action_header *a = (struct ofl_action_header*) xmalloc(sizeof(struct ofl_action_header));
    acts[act_num] = (struct ofl_action_header*) a;
    acts[act_num]->type = OFPAT_POP_VLAN;
    act_num++;    
}
    
void 
Actions::CreatePopMpls(uint16_t ethertype){
    
    if (act_num)
        acts = (struct ofl_action_header**) xrealloc(acts, sizeof(struct ofl_action_header *) * (act_num +1)); 
    struct ofl_action_pop_mpls *a = (struct ofl_action_pop_mpls*) xmalloc(sizeof(struct ofl_action_pop_mpls));
    a->ethertype = ethertype;
    acts[act_num] = (struct ofl_action_header*) a;
    acts[act_num]->type = OFPAT_POP_MPLS;
    act_num++;    
}
    
void 
Actions::CreateSetNwTTL(uint8_t nw_ttl){
    
    if (act_num)
        acts = (struct ofl_action_header**) xrealloc(acts, sizeof(struct ofl_action_header *) * (act_num +1)); 
    struct ofl_action_set_nw_ttl *a = (struct ofl_action_set_nw_ttl*) xmalloc(sizeof(struct ofl_action_set_nw_ttl));
    a->nw_ttl = nw_ttl;
    acts[act_num] = (struct ofl_action_header*) a;
    acts[act_num]->type = OFPAT_SET_NW_TTL;
    act_num++;
}

void 
Actions::CreateSetMplsTTL(uint8_t mpls_ttl){
    acts = (struct ofl_action_header**) xrealloc(acts, sizeof(struct ofl_action_header *) * act_num);
    struct ofl_action_mpls_ttl *a = (struct ofl_action_mpls_ttl*) xmalloc(sizeof(struct ofl_action_mpls_ttl));
    a->mpls_ttl = mpls_ttl;
    acts[act_num] = (struct ofl_action_header*) a;
    acts[act_num]->type = OFPAT_SET_MPLS_TTL;
    act_num++;    
}
    
void 
Actions::CreateSetQueue(uint32_t queue_id){
    
    if (act_num)
        acts = (struct ofl_action_header**) xrealloc(acts, sizeof(struct ofl_action_header *) * (act_num +1)); 
    struct ofl_action_set_queue *a = (struct ofl_action_set_queue*) xmalloc(sizeof(struct ofl_action_set_queue));
    a->queue_id = queue_id;
    acts[act_num] = (struct ofl_action_header*) a;
    acts[act_num]->type = OFPAT_SET_QUEUE;
    act_num++;  
}
    
void 
Actions::CreateGroupAction(uint32_t group_id){
    
    if (act_num)
        acts = (struct ofl_action_header**) xrealloc(acts, sizeof(struct ofl_action_header *) * (act_num +1)); 
    struct ofl_action_group *a = (struct ofl_action_group*) xmalloc(sizeof(struct ofl_action_group));
    a->group_id = group_id;
    acts[act_num] = (struct ofl_action_header*) a;
    acts[act_num]->type = OFPAT_GROUP;
    act_num++;      
}

} //namespace vigil
