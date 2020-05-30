
header_type cheetah_t {
    fields {
        flowId : 16;
        rowId : 32;
        value_hi : 32;
        value_lo : 32;
    }
}

header_type cheetah_md_t {
    fields {
        rowId : 32;
        prune : 8;
        replace_value_lo : 32;
        replace_value_hi : 32;
    }
}   

metadata cheetah_md_t cheetah_md;
metadata cheetah_md_t cheetah_md_2;




header cheetah_t cheetah;

parser parse_cheetah {
    extract(cheetah);
    set_metadata(cheetah_md.rowId, cheetah.rowId);
    return ingress;
}   




action prune() {
    drop();
}

action copy_to_metadata_action() {
    modify_field(cheetah_md.replace_value_lo, cheetah.value_lo);
    modify_field(cheetah_md.replace_value_hi, cheetah.value_hi);
}

table metadata_copy_table {
    reads {
        cheetah.flowId : exact;
    }

    actions {
        copy_to_metadata_action;
    }

    default_action : copy_to_metadata_action();
}

field_list lru_cache_hash_fields {
    cheetah.value_lo;
    cheetah.value_hi;
}

field_list_calculation lru_cache_hash {
    input { lru_cache_hash_fields; }
    algorithm : crc32;
    output_width : 16;
}

#include "cheetah_lru_import_efficient.p4"
#include "cheetah_lru_import_efficient_2.p4"
#include "cheetah_lru_import_efficient_3.p4"
#include "cheetah_lru_import_efficient_4.p4"

control ingress {
    apply(metadata_copy_table);

    apply(lru_cache_lo);
    apply(lru_cache_hi);

    if ((cheetah_md_2.replace_value_lo & 0x80000000 == 0x80000000) and (cheetah_md_2.replace_value_hi & 0x80000000 == 0x80000000)) {
        apply(prune_table);
    }

    else {
        apply(lru_cache_lo_2);
        apply(lru_cache_hi_2);
    
        if ((cheetah_md.replace_value_lo & 0x80000000 == 0x80000000) and (cheetah_md.replace_value_hi & 0x80000000 == 0x80000000)) {
            apply(prune_table_2);
        }

        else {
            apply(lru_cache_lo_3);
            apply(lru_cache_hi_3);
 
            if ((cheetah_md_2.replace_value_lo & 0x80000000 == 0x80000000) and (cheetah_md_2.replace_value_hi & 0x80000000 == 0x80000000)) {
                apply(prune_table_3);
            }

            else {
                apply(lru_cache_lo_4);
                apply(lru_cache_hi_4);

                if ((cheetah_md.replace_value_lo & 0x80000000 == 0x80000000) and (cheetah_md.replace_value_hi & 0x80000000 == 0x80000000)) {
                    apply(prune_table_4);
                }
            }
        }
    }
}

