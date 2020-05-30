#include <tofino/intrinsic_metadata.p4>
#include <tofino/constants.p4>
#include <tofino/stateful_alu_blackbox.p4>

header_type cheetah_t {
    fields {
        flowId : 16;
        rowId : 32;
        value : 32;
    }
}

header_type cheetah_md_t {
    fields {
        rowId : 32;
        diff : 32;
        process : 8;
        prune : 8;
        topn_min_val_one : 32;
        topn_less_than_min : 8;
        topn_more_than_n : 8;
        topn_less_than_min_two : 8;
    }
}   

metadata cheetah_md_t cheetah_md;

header cheetah_t cheetah;

parser parse_cheetah {
    extract(cheetah);
    set_metadata(cheetah_md.rowId, cheetah.rowId);
    return ingress;
}   

#define GENERATE_TOPN_MIN_ALU(name, reg_name, store_dest)\
blackbox stateful_alu cheetah_topn_query_ ## name {\
    reg : reg_name;\
    \
    condition_lo : register_lo - cheetah.value > 0;\
    \
    update_lo_1_predicate : condition_lo;\
    update_lo_1_value : cheetah.value;\
    update_lo_2_predicate : not condition_lo;\
    update_lo_2_value : register_lo;\
\
    update_hi_1_predicate : condition_lo;\
    update_hi_1_value : 1;\
    update_hi_2_predicate : not condition_lo;\
    update_hi_2_value : 0;\
\
    output_value : alu_hi;\
    output_dst : store_dest;\
}

register cheetah_topn_counter_reg {
    width : 32;
    instance_count : 1;
}

register cheetah_topn_min_reg_one {
    width : 32;
    instance_count : 1;
}

register cheetah_topn_pack_reg {
    width : 32;
    instance_count : 1;
}

register cheetah_topn_counter_two_reg {
    width : 32;
    instance_count : 1;
}

#define GENERATE_PACKET_COUNTER_ALU(name, reg_name, cutoff, store_dest)\
blackbox stateful_alu cheetah_topn_count_pkt_ ## name {\
    reg : reg_name;\
\
    condition_lo : register_lo < cutoff;\
\
    update_lo_1_predicate : condition_lo;\
    update_lo_1_value : register_lo + 1;\
    update_lo_2_predicate : not condition_lo;\
    update_lo_2_value : register_lo;\
\
    update_hi_1_predicate : condition_lo;\
    update_hi_1_value : 0;\
    update_hi_2_predicate : not condition_lo;\
    update_hi_2_value : 1;\
\
    output_value : alu_hi;\
    output_dst : store_dest;\
}

GENERATE_PACKET_COUNTER_ALU(one, cheetah_topn_counter_reg, 
                                CHEETAH_TOPN_VALUE, cheetah_md.topn_more_than_n)

GENERATE_TOPN_MIN_ALU(one, cheetah_topn_min_reg_one, cheetah_md.topn_less_than_min)

blackbox stateful_alu cheetah_topn_min_pack {
    reg : cheetah_topn_min_reg_one;
    
    update_lo_1_value : register_lo;
    output_value : alu_lo;
    output_dst : cheetah_md.topn_min_val_one;
}

blackbox stateful_alu cheetah_topn_t_one {
    reg : cheetah_topn_counter_two_reg;

    condition_lo : cheetah_md.topn_min_val_one > CHEETAH_TOPN_INT_MAX;
    condition_hi : register_lo - CHEETAH_TOPN_VALUE > 0;    

    update_lo_1_predicate : condition_lo and not condition_hi;
    update_lo_1_value : register_lo + 1;

    update_hi_1_predicate : condition_lo and condition_hi;
    update_hi_1_value : 1;
    update_hi_2_predicate : not condition_lo or not condition_hi;
    update_hi_2_value : 0;    

    output_value : alu_hi;
    output_dst : cheetah_md.topn_less_than_min_two;
}

action cheetah_topn_count(index) {
    cheetah_topn_count_pkt_one.execute_stateful_alu(index);
}

action cheetah_topn_min_action(index) {
    cheetah_topn_query_one.execute_stateful_alu(index);
}

action cheetah_topn_min_pack_action(index) {
    cheetah_topn_min_pack.execute_stateful_alu(index);
}

action cheetah_topn_t_one_action(index) { 
    cheetah_topn_t_one.execute_stateful_alu(index);
}

action cheetah_topn_debug_one_action(index) {
    cheetah_topn_debug_one.execute_stateful_alu(index);
}

action prune() {
    drop();
}

action cheetah_topn_double_action() {
    shift_left(cheetah_md.topn_min_val_one, cheetah_md.topn_min_val_one, 1);
}

action cheetah_topn_subtract_action() {
    subtract_from_field(cheetah_md.topn_min_val_one, cheetah.value);
}

table cheetah_topn_table_count {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        cheetah_topn_count;
    }

    default_action : cheetah_topn_count(0);
}

table cheetah_topn_table_min {
    reads {
        cheetah.flowId : exact;
        cheetah_md.topn_more_than_n : exact;
    }

    actions {
        cheetah_topn_min_action;
        cheetah_topn_min_pack_action;
    }

    default_action : cheetah_topn_min_action(0);
}

table cheetah_topn_double {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        cheetah_topn_double_action;
    }

    default_action : cheetah_topn_double_action();
}

table cheetah_topn_subtract {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        cheetah_topn_subtract_action;
        nop;
    }

    default_action : cheetah_topn_subtract_action();
}

table cheetah_topn_t_one_table {
    reads {
        cheetah.flowId : exact;
        cheetah_md.topn_more_than_n : exact;
    }

    actions {
        cheetah_topn_t_one_action;
        nop;
    }

    default_action : nop;
}

table cheetah_topn_prune_check {
    reads {
        cheetah_md.topn_less_than_min : exact;
        cheetah_md.topn_less_than_min_two : exact;
        cheetah_md.topn_more_than_n : exact;
    }

    actions {
        nop;
        prune;
    }

    default_action : nop;
}

control ingress {
    apply(cheetah_topn_table_count);
    apply(cheetah_topn_table_min);
    apply(cheetah_topn_double);
    apply(cheetah_topn_subtract);
    apply(cheetah_topn_t_one_table);
    apply(cheetah_topn_prune_check);
}
