
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
        not_in_bloom : 1;
        key_1 : 32;
        key_2 : 32;
        key_3 : 32;
        key_4 : 32;
        key_5 : 32;
        key_6 : 32;
    }
}   

metadata cheetah_md_t cheetah_md;

header cheetah_t cheetah;

parser parse_cheetah {
    extract(cheetah);
    return ingress;
}   

action prune() {
    drop();
}

#define CREATE_BLOOM(N)\
register reg_bloom_##N {                \
    width : 1;                          \
    instance_count : REGISTER_COUNT_MAX;\
}                                       \
                                        \
blackbox stateful_alu alu_bloom_##N {   \
    reg : reg_bloom_##N;                \
    update_lo_1_value: set_bitc;        \
    output_value : alu_lo;              \
    output_dst : cheetah_md.not_in_bloom;   \
}                                       \
                                        \
blackbox stateful_alu alu_bloom_##N##_r { \
    reg : reg_bloom_##N;                \
    update_lo_1_value : read_bit;       \
    output_value : alu_lo;              \
    output_dst : cheetah_md.not_in_bloom;   \
}                                       \
                                        \
field_list list_##N {                   \
    cheetah_md.key_##N;                 \
}                                       \
                                        \
field_list_calculation hash_bloom_##N { \
    input {                             \
        list_##N;                       \
    }                                   \
    algorithm : crc32;                  \
    output_width : 17;                  \
}                                       \
                                        \
action act_bloom_##N() {                \
    alu_bloom_##N.execute_stateful_alu_from_hash(hash_bloom_##N);  \
}                                       \
                                        \
action act_bloom_##N##_r() {            \
    alu_bloom_##N##_r.execute_stateful_alu_from_hash(hash_bloom_##N);\
}                                       \
                                        \
table tbl_bloom_##N {                   \
    reads {                             \
        cheetah.flowId : exact;         \
    }                                   \
                                        \
    actions {                           \
        act_bloom_##N;                  \
        act_bloom_##N##_r;                \
    }                                   \
}                                       

CREATE_BLOOM(1)
CREATE_BLOOM(2)
CREATE_BLOOM(3)
CREATE_BLOOM(4)
CREATE_BLOOM(5)
CREATE_BLOOM(6)

action act_prepare_keys() {
    modify_field(cheetah_md.key_1, cheetah.key);
    modify_field(cheetah_md.key_2, cheetah.key);
    modify_field(cheetah_md.key_3, cheetah.key);
    modify_field(cheetah_md.key_4, cheetah.key);
    modify_field(cheetah_md.key_5, cheetah.key);
    modify_field(cheetah_md.key_6, cheetah.key);
}

table tbl_prepare_keys {
    reads {
        udp.dstPort : exact;
    }

    actions {
        act_prepare_keys;
    }
}

action act_prepare_keys_2() {
    add_to_field(cheetah_md.key_2, 3);
    add_to_field(cheetah_md.key_3, 5);
    add_to_field(cheetah_md.key_4, 7);
    add_to_field(cheetah_md.key_5, 11);
    add_to_field(cheetah_md.key_6, 13);
}

table tbl_prepare_keys_2 {
    reads {
        udp.dstPort : exact;
    }

    actions {
        act_prepare_keys_2;
    }
}

table tbl_prune {
    reads {
        udp.dstPort : exact;
    }

    actions {
        prune;
    }
}

control ingress {
    apply(tbl_prepare_keys);
    apply(tbl_prepare_keys_2);

    apply(tbl_bloom_1);

    if (cheetah_md.not_in_bloom == 1) {
        apply(tbl_prune);
    } else {
        apply(tbl_bloom_2);

        if (cheetah_md.not_in_bloom == 1) {
            apply(tbl_prune);
        } else {
            apply(tbl_bloom_3);

            if (cheetah_md.not_in_bloom == 1) {
                apply(tbl_prune);
            } else {
                apply(tbl_bloom_4);

                if (cheetah_md.not_in_bloom == 1) {
                    apply(tbl_prune);
                } else {
                    apply(tbl_bloom_5);
                    
                    if (cheetah_md.not_in_bloom == 1) {
                        apply(tbl_prune);
                    } else {
                        apply(tbl_bloom_6);

                        if (cheetah_md.not_in_bloom == 1) {
                            apply(tbl_prune);
                        }
                    }
                }
            }
        }
    }
}

