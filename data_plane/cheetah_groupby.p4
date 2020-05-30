
header_type cheetah_t {
    fields {
        flowId : 16;
        rowId : 32;
        key : 32;
        value : 32;
    }
}

header_type cheetah_md_t {
    fields {
        prune : 8;
        sketch_one_same_key : 8;
        sketch_two_same_key : 8;
        sketch_three_same_key : 8;
        sketch_four_same_key : 8;
        sketch_five_same_key : 8;
        hash_value : 32;
    }
}   

metadata cheetah_md_t cheetah_md;

header cheetah_t cheetah;

parser parse_cheetah {
    extract(cheetah);
    return ingress;
}   

action set_egr(egress_spec) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_spec);
}

action nop() {
}

action _drop() {
    drop();
}

action prune() {
    drop();
}

register reg_sketch_one_key {
    width : 32;
    instance_count : REGISTER_COUNT_MAX;
}

blackbox stateful_alu alu_sketch_one_key {
    reg : reg_sketch_one_key;

    condition_lo : cheetah.key - register_lo == 0;

    update_lo_1_predicate : not condition_lo;
    update_lo_1_value : cheetah.key;

    update_hi_1_predicate : condition_lo;
    update_hi_1_value : 1;
    update_hi_2_predicate : not condition_lo;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.sketch_one_same_key;
}

register reg_sketch_one_value {
    width : 32;
    instance_count : REGISTER_COUNT_MAX;
}

blackbox stateful_alu alu_sketch_one_value {
    reg : reg_sketch_one_value;

    condition_lo : cheetah_md.sketch_one_same_key == 1;
    condition_hi : register_lo - cheetah.value > 0;

    update_lo_1_predicate : not condition_lo or not condition_hi;
    update_lo_1_value : cheetah.value;

    update_hi_1_predicate : condition_lo and condition_hi;
    update_hi_1_value : 1;
    update_hi_2_predicate : condition_lo or not condition_hi;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.prune;
}

field_list hash_list_one {
    cheetah_md.hash_value;
}

field_list_calculation hash_one {
    input { 
        hash_list_one;
    }

    algorithm : crc32;
    output_width : 32;
}

action action_sketch_one_key() {
    alu_sketch_one_key.execute_stateful_alu_from_hash(hash_one);
}


action action_sketch_one_value() {
    alu_sketch_one_value.execute_stateful_alu_from_hash(hash_one);
}



table table_sketch_one_key {
    reads {
        cheetah.flowId : exact;
    }
    
    actions {
        action_sketch_one_key;
        nop;
    }
}


table table_sketch_one_value {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_sketch_one_value;
        nop;
    }
}  


register reg_sketch_two_key {
    width : 32;
    instance_count : REGISTER_COUNT_MAX;
}


blackbox stateful_alu alu_sketch_two_key {
    reg : reg_sketch_two_key;

    condition_lo : cheetah.key - register_lo == 0;

    update_lo_1_predicate : not condition_lo;
    update_lo_1_value : cheetah.key;

    update_hi_1_predicate : condition_lo;
    update_hi_1_value : 1;
    update_hi_2_predicate : not condition_lo;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.sketch_two_same_key;
}


register reg_sketch_two_value {
    width : 32;
    instance_count : REGISTER_COUNT_MAX;
}


blackbox stateful_alu alu_sketch_two_value {
    reg : reg_sketch_two_value;

    condition_lo : cheetah_md.sketch_two_same_key == 1;
    condition_hi : register_lo - cheetah.value > 0;

    update_lo_1_predicate : not condition_lo or not condition_hi;
    update_lo_1_value : cheetah.value;

    update_hi_1_predicate : condition_lo and condition_hi;
    update_hi_1_value : 1;
    update_hi_2_predicate : condition_lo or not condition_hi;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.prune;
}

field_list hash_list_two {
    cheetah_md.hash_value;
}


field_list_calculation hash_two {
    input {
        hash_list_two;
    }

    algorithm : crc32;
    output_width : 17;
}


action action_sketch_two_key() {
    alu_sketch_two_key.execute_stateful_alu_from_hash(hash_two);
}


action action_sketch_two_value() {
    alu_sketch_two_value.execute_stateful_alu_from_hash(hash_two);
}


table table_sketch_two_key {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_sketch_two_key;
        nop;
    }    
}


table table_sketch_two_value {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_sketch_two_value;
        nop;
    }
}


register reg_sketch_three_key {
    width : 32;
    instance_count : REGISTER_COUNT_MAX;
}


blackbox stateful_alu alu_sketch_three_key {
    reg : reg_sketch_three_key;

    condition_lo : cheetah.key - register_lo == 0;

    update_lo_1_predicate : not condition_lo;
    update_lo_1_value : cheetah.key;

    update_hi_1_predicate : condition_lo;
    update_hi_1_value : 1;
    update_hi_2_predicate : not condition_lo;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.sketch_three_same_key;
}


register reg_sketch_three_value {
    width : 32;
    instance_count : REGISTER_COUNT_MAX;
}


blackbox stateful_alu alu_sketch_three_value {
    reg : reg_sketch_three_value;

    condition_lo : cheetah_md.sketch_three_same_key == 1;
    condition_hi : register_lo - cheetah.value > 0;

    update_lo_1_predicate : not condition_lo or not condition_hi;
    update_lo_1_value : cheetah.value;

    update_hi_1_predicate : condition_lo and condition_hi;
    update_hi_1_value : 1;
    update_hi_2_predicate : condition_lo or not condition_hi;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.prune;
}

field_list hash_list_three {
    cheetah_md.hash_value;
}


field_list_calculation hash_three {
    input {
        hash_list_three;
    }

    algorithm : crc32;
    output_width : 17;
}


action action_sketch_three_key() {
    alu_sketch_three_key.execute_stateful_alu_from_hash(hash_three);
}


action action_sketch_three_value() {
    alu_sketch_three_value.execute_stateful_alu_from_hash(hash_three);
}


table table_sketch_three_key {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_sketch_three_key;
        nop;
    }    
}


table table_sketch_three_value {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_sketch_three_value;
        nop;
    }
}


register reg_sketch_four_key {
    width : 32;
    instance_count : REGISTER_COUNT_MAX;
}


blackbox stateful_alu alu_sketch_four_key {
    reg : reg_sketch_four_key;

    condition_lo : cheetah.key - register_lo == 0;

    update_lo_1_predicate : not condition_lo;
    update_lo_1_value : cheetah.key;

    update_hi_1_predicate : condition_lo;
    update_hi_1_value : 1;
    update_hi_2_predicate : not condition_lo;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.sketch_four_same_key;
}


register reg_sketch_four_value {
    width : 32;
    instance_count : REGISTER_COUNT_MAX;
}


blackbox stateful_alu alu_sketch_four_value {
    reg : reg_sketch_four_value;

    condition_lo : cheetah_md.sketch_four_same_key == 1;
    condition_hi : register_lo - cheetah.value > 0;

    update_lo_1_predicate : not condition_lo or not condition_hi;
    update_lo_1_value : cheetah.value;

    update_hi_1_predicate : condition_lo and condition_hi;
    update_hi_1_value : 1;
    update_hi_2_predicate : condition_lo or not condition_hi;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.prune;
}

field_list hash_list_four {
    cheetah_md.hash_value;
}


field_list_calculation hash_four {
    input {
        hash_list_four;
    }

    algorithm : crc32;
    output_width : 17;
}


action action_sketch_four_key() {
    alu_sketch_four_key.execute_stateful_alu_from_hash(hash_four);
}


action action_sketch_four_value() {
    alu_sketch_four_value.execute_stateful_alu_from_hash(hash_four);
}


table table_sketch_four_key {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_sketch_four_key;
        nop;
    }    
}


table table_sketch_four_value {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_sketch_four_value;
        nop;
    }
}


register reg_sketch_five_key {
    width : 32;
    instance_count : REGISTER_COUNT_MAX;
}


blackbox stateful_alu alu_sketch_five_key {
    reg : reg_sketch_five_key;

    condition_lo : cheetah.key - register_lo == 0;

    update_lo_1_predicate : not condition_lo;
    update_lo_1_value : cheetah.key;

    update_hi_1_predicate : condition_lo;
    update_hi_1_value : 1;
    update_hi_2_predicate : not condition_lo;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.sketch_five_same_key;
}


register reg_sketch_five_value {
    width : 32;
    instance_count : REGISTER_COUNT_MAX;
}


blackbox stateful_alu alu_sketch_five_value {
    reg : reg_sketch_five_value;

    condition_lo : cheetah_md.sketch_five_same_key == 1;
    condition_hi : register_lo - cheetah.value > 0;

    update_lo_1_predicate : not condition_lo or not condition_hi;
    update_lo_1_value : cheetah.value;

    update_hi_1_predicate : condition_lo and condition_hi;
    update_hi_1_value : 1;
    update_hi_2_predicate : condition_lo or not condition_hi;
    update_hi_2_value : 0;

    output_value : alu_hi;
    output_dst : cheetah_md.prune;
}

field_list hash_list_five {
    cheetah_md.hash_value;
}


field_list_calculation hash_five {
    input {
        hash_list_five;
    }

    algorithm : crc32;
    output_width : 17;
}


action action_sketch_five_key() {
    alu_sketch_five_key.execute_stateful_alu_from_hash(hash_five);
}


action action_sketch_five_value() {
    alu_sketch_five_value.execute_stateful_alu_from_hash(hash_five);
}


table table_sketch_five_key {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_sketch_five_key;
        nop;
    }    
}


table table_sketch_five_value {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        action_sketch_five_value;
        nop;
    }
}

action modify_hash() {
    modify_field(cheetah_md.hash_value, cheetah.key);
}

action modify_hash_two() {
    add_to_field(cheetah_md.hash_value, 3);
}

action modify_hash_three() {
    add_to_field(cheetah_md.hash_value, 2);
}

action modify_hash_four() {
    add_to_field(cheetah_md.hash_value, 2);
}

action modify_hash_five() {
    add_to_field(cheetah_md.hash_value, 2);
}


table table_hash_init {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        modify_hash;
        nop;
    }
}


table table_hash_init_two {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        modify_hash_two;
        nop;
    }
}


table table_hash_init_three {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        modify_hash_three;
        nop;
    }
}


table table_hash_init_four {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        modify_hash_four;
        nop;
    }
}


table table_hash_init_five {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        modify_hash_five;
        nop;
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
    apply(table_hash_init);
   
    apply(table_sketch_one_key);
    apply(table_sketch_one_value);

    apply(table_hash_init_two);

    if (cheetah_md.prune == 0x0) {
        apply(table_sketch_two_key);
        apply(table_sketch_two_value);
    }
    
    apply(table_hash_init_three);

    if (cheetah_md.prune == 0x0) {
        apply(table_sketch_three_key);
        apply(table_sketch_three_value);
    }

    apply(table_hash_init_four);

    if (cheetah_md.prune == 0x0) {
        apply(table_sketch_four_key);
        apply(table_sketch_four_value);
    }

    apply(table_hash_init_five);

    if (cheetah_md.prune == 0x0) {
        apply(table_sketch_five_key);
        apply(table_sketch_five_value);
    }

    if (cheetah_md.prune == 0x1) {
        apply(table_prune);
    }
}
