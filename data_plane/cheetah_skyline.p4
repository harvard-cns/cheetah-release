#include <tofino/intrinsic_metadata.p4>
#include <tofino/constants.p4>
#include <tofino/stateful_alu_blackbox.p4>

header_type cheetah_t {
    fields {
        flowId : 16;
        rowId : 32;
        x : 32;
        y : 32;
    }
}

header_type cheetah_md_t {
    fields {
        prune : 8;
        replace_next : 8;
        val_sum : 32;
        dominates_x : 8;
        dominates_y : 8;
    }
}

header cheetah_t cheetah;

metadata cheetah_md_t cheetah_md;

parser parse_cheetah {
    extract(cheetah);
    set_metadata(cheetah_md.val_sum, cheetah.x);
    return ingress;
}   

action set_egr(egress_spec) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_spec);
}

action prune() {
    drop();
}

action action_heuristic_pkt() {
    add_to_field(cheetah_md.val_sum, cheetah.y);
}

table table_heuristic_pkt {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_heuristic_pkt;
    }
}

register reg_heuristic {
    width : 32;
    instance_count : 1;
}

blackbox stateful_alu alu_heuristic {
    reg : reg_heuristic;

    condition_lo : register_lo - cheetah_md.val_sum < 0;
    update_lo_1_predicate : condition_lo;
    update_lo_1_value : cheetah_md.val_sum;

    update_hi_1_predicate : condition_lo;
    update_hi_1_value : 1;
    update_hi_2_predicate : not condition_lo;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.replace_next;
}

action action_heuristic() { 
    alu_heuristic.execute_stateful_alu(0);
}


table table_heuristic {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_heuristic;
    }
}

register reg_x {
    width : 32;
    instance_count : 1;
}

blackbox stateful_alu alu_x {
    reg : reg_x;

    condition_lo : cheetah_md.replace_next == 1;
    condition_hi : register_lo - cheetah.x > 0;
    
    update_lo_1_predicate : condition_lo;
    update_lo_1_value : cheetah.x;

    update_hi_1_predicate : condition_hi and not condition_lo;
    update_hi_1_value : 1;
    update_hi_2_predicate : not condition_hi or condition_lo;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.dominates_x;
}

action action_x () {
    alu_x.execute_stateful_alu(0);
}

table table_x {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_x;
    }
}

register reg_y {
    width : 32;
    instance_count : 1;
}

blackbox stateful_alu alu_y {
    reg : reg_y;

    condition_lo : cheetah_md.replace_next == 1;
    condition_hi : register_lo - cheetah.y > 0;
    
    update_lo_1_predicate : condition_lo;
    update_lo_1_value : cheetah.y;

    update_hi_1_predicate : condition_hi and not condition_lo;
    update_hi_1_value : 1;
    update_hi_2_predicate : not condition_hi or condition_lo;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.dominates_y;
}

action action_y() {
    alu_y.execute_stateful_alu(0);
}

table table_y {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_y;
    }
}

table forward {
    reads {
        ethernet.dstAddr : exact;
    }
    actions {
        set_egr; nop;
    }
    default_action: set_egr(40);
}

table acl {
    reads {
        ethernet.dstAddr : ternary;
        ethernet.srcAddr : ternary;
    }
    actions {
        nop;
        _drop;
    }
}

table table_prune {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        prune;
    }
}

control ingress {
    apply(table_heuristic_pkt);
    apply(table_heuristic);
    apply(table_x);
    apply(table_y);
    if (cheetah_md.dominates_x == 0x1 and cheetah_md.dominates_y == 0x1) {
        apply(table_prune);
    }
}
