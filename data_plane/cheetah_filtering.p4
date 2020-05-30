
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
        prune : 8;
    }
}   

metadata cheetah_md_t cheetah_md;

header cheetah_t cheetah;

parser parse_cheetah {
    extract(cheetah);
    set_metadata(cheetah_md.rowId, cheetah.rowId);
    return ingress;
}   

action prune() {
    drop();
}

table table_prune {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        prune;
    }
}

register filter_reg { 
    width : 32;
    instance_count : 1;
}

blackbox stateful_alu alu_filter {
    reg : filter_reg;

    condition_lo : cheetah.value < FILTER_CONSTANT;
    
    update_hi_1_predicate : condition_lo;
    update_hi_1_value : 0;
    update_hi_2_predicate: not condition_lo;
    update_hi_2_value : 1;

    output_value : alu_hi;
    output_dst : cheetah_md.prune;
}

action action_filter() {
    alu_filter.execute_stateful_alu(0);
}

table table_filter {
    reads { 
        cheetah.flowId : exact;
    }

    actions {
        action_filter;
    }
}

control ingress {
    apply(table_filter);
    if (cheetah_md.prune == 0x1) {
        apply(table_prune);
    }
}

