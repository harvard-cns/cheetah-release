#ifndef CHEETAH_LRU_IMPORT_P4
#define CHEETAH_LRU_IMPORT_P4

register lru_reg_lo {
    width : 32;
    instance_count : LRU_WIDTH;
}

register lru_reg_hi { 
    width : 32;
    instance_count : LRU_WIDTH;
}

blackbox stateful_alu lru_one_lo {
    reg : lru_reg_lo;
    
    condition_lo : cheetah.value_lo - register_lo == 0;

    update_lo_1_value : cheetah_md.replace_value_lo;
    
    update_hi_1_predicate : condition_lo;
    update_hi_1_value : register_lo | 0x80000000;
    update_hi_2_predicate : not condition_lo;
    update_hi_2_value : register_lo;

    output_value : alu_hi;
    output_dst : cheetah_md_2.replace_value_lo;
}

blackbox stateful_alu lru_one_hi {
    reg : lru_reg_hi;

    condition_lo : cheetah.value_hi - register_lo == 0;

    update_lo_1_value : cheetah_md.replace_value_hi;
    
    update_hi_1_predicate : condition_lo;
    update_hi_1_value : register_lo | 0x80000000;
    update_hi_2_predicate : not condition_lo;
    update_hi_2_value : register_lo;
    
    output_value : alu_hi;
    output_dst : cheetah_md_2.replace_value_hi;
}

action lru_action_lo() {
    lru_one_lo.execute_stateful_alu_from_hash(lru_cache_hash);
}

action lru_action_hi() {
    lru_one_hi.execute_stateful_alu_from_hash(lru_cache_hash);
}

table lru_cache_lo {
    reads { 
        cheetah.flowId : exact;
    }

    actions {
        lru_action_lo;
    }
}

table lru_cache_hi {
    reads {
        cheetah.flowId : exact;
    }

    actions { 
        lru_action_hi;
    }
}

table prune_table {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        prune;
    }

    default_action : prune;
}

#endif /* CHEETAH_LRU_IMPORT_P4 */
