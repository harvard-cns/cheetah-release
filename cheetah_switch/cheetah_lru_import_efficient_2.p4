#ifndef CHEETAH_LRU_IMPORT_P4_2
#define CHEETAH_LRU_IMPORT_P4_2

register lru_reg_lo_2 {
    width : 32;
    instance_count : LRU_WIDTH;
}

register lru_reg_hi_2 { 
    width : 32;
    instance_count : LRU_WIDTH;
}

blackbox stateful_alu lru_one_lo_2 {
    reg : lru_reg_lo_2;
    
    condition_lo : cheetah.value_lo - register_lo == 0;

    update_lo_1_value : cheetah_md_2.replace_value_lo;
    
    update_hi_1_predicate : condition_lo;
    update_hi_1_value : register_lo | 0x80000000;
    update_hi_2_predicate : not condition_lo;
    update_hi_2_value : register_lo;

    output_value : alu_hi;
    output_dst : cheetah_md.replace_value_lo;
}

blackbox stateful_alu lru_one_hi_2 {
    reg : lru_reg_hi_2;

    condition_lo : cheetah.value_hi - register_lo == 0;

    update_lo_1_value : cheetah_md_2.replace_value_hi;
    
    update_hi_1_predicate : condition_lo;
    update_hi_1_value : register_lo | 0x80000000;
    update_hi_2_predicate : not condition_lo;
    update_hi_2_value : register_lo;
    
    output_value : alu_hi;
    output_dst : cheetah_md.replace_value_hi;
}

action lru_action_lo_2() {
    lru_one_lo_2.execute_stateful_alu_from_hash(lru_cache_hash);
}

action lru_action_hi_2() {
    lru_one_hi_2.execute_stateful_alu_from_hash(lru_cache_hash);
}

table lru_cache_lo_2 {
    reads { 
        cheetah.flowId : exact;
    }

    actions {
        lru_action_lo_2;
    }
}

table lru_cache_hi_2 {
    reads {
        cheetah.flowId : exact;
    }

    actions { 
        lru_action_hi_2;
    }
}

table prune_table_2 {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        prune;
    }

    default_action : prune;
}

#endif /* CHEETAH_LRU_IMPORT_P4_2 */

