# Control Plane Rules For Cheetah Queries

## Filtering
     pd cheetah_filter_table add_entry cheetah_filter cheetah_flowId 1 action_index 1
     pd cheetah_filter_table add_entry cheetah_filter cheetah_flowId 0 action_index 0
     pd cheetah_prune_check add_entry prune cheetah_md_prune 1
     pd cheetah_prune_check add_entry nop cheetah_md_prune 0
	

## Distinct
     pd lru_cache_hi add_entry lru_action_hi cheetah_flowId 1 
     pd lru_cache_lo add_entry lru_action_lo cheetah_flowId 1 
     pd lru_cache_lo_2 add_entry lru_action_lo_2 cheetah_flowId 1
     pd lru_cache_hi_2 add_entry lru_action_hi_2 cheetah_flowId 1
     pd lru_cache_lo_3 add_entry lru_action_lo_3 cheetah_flowId 1
     pd lru_cache_hi_3 add_entry lru_action_hi_3 cheetah_flowId 1
     pd lru_cache_lo_4 add_entry lru_action_lo_4 cheetah_flowId 1
     pd lru_cache_hi_4 add_entry lru_action_hi_4 cheetah_flowId 1
     pd prune_table add_entry prune cheetah_flowId 1
     pd prune_table_2 add_entry prune cheetah_flowId 1
     pd metadata_copy_table add_entry copy_to_metadata_action cheetah_flowId 1
	

## Deterministic Top-N - 4 cutoffs (t0 to t3)
     pd cheetah_topn_table_count add_entry cheetah_topn_count cheetah_flowId 1 action_index 1
     pd cheetah_topn_table_min add_entry cheetah_topn_min_action cheetah_flowId 1 cheetah_md_topn_more_than_n 0 action_index 1
     pd cheetah_topn_table_min add_entry cheetah_topn_min_pack_a_action cheetah_flowId 1 cheetah_md_topn_more_than_n 1 action_index 1
     pd cheetah_topn_table_min_appendix_two add_entry cheetah_topn_min_pack_b_action cheetah_flowId 1 cheetah_md_topn_more_than_n 1 action_index 1
     pd cheetah_topn_t_one_table add_entry cheetah_topn_t_one_action cheetah_flowId 1 cheetah_md_topn_more_than_n 1 action_index 1
     pd cheetah_topn_t_one_table_check add_entry cheetah_topn_t_one_check_action cheetah_flowId 1 cheetah_md_topn_t1_cutoff_reached 1 action_index 1
     pd cheetah_topn_t_two_table add_entry cheetah_topn_t_two_action cheetah_flowId 1 cheetah_md_topn_t1_cutoff_reached 1 action_index 1
     pd cheetah_topn_t_two_table_check add_entry cheetah_topn_t_two_check_action cheetah_flowId 1 cheetah_md_topn_t2_cutoff_reached 1 action_index 1
     pd cheetah_topn_t_three_table add_entry cheetah_topn_t_three_action cheetah_flowId 1 cheetah_md_topn_t2_cutoff_reached 1 action_index 1
     pd cheetah_topn_t_three_table_check add_entry cheetah_topn_t_three_check_action cheetah_flowId 1 cheetah_md_topn_t3_cutoff_reached 1 action_index 1
     pd cheetah_topn_prune_check add_entry prune cheetah_md_prune 1
pd register_write cheetah_topn_min_reg_one index 1 register_value 10000
	



## Top-N
    pd cheetah_topn_table_min add_entry cheetah_topn_min_action cheetah_flowId 1 cheetah_md_topn_more_than_n 0 action_index 1
    pd cheetah_topn_table_min add_entry cheetah_topn_min_pack_action cheetah_flowId 1 cheetah_md_topn_more_than_n 1 action_index 1
    pd cheetah_topn_prune_check add_entry prune cheetah_md_topn_less_than_min 0 cheetah_md_topn_less_than_min_two 1 cheetah_md_topn_more_than_n 1
    pd cheetah_topn_prune_check add_entry prune cheetah_md_topn_less_than_min 1 cheetah_md_topn_less_than_min_two 1 cheetah_md_topn_more_than_n 1
    pd cheetah_topn_prune_check add_entry prune cheetah_md_topn_less_than_min 1 cheetah_md_topn_less_than_min_two 0 cheetah_md_topn_more_than_n 1
    pd cheetah_topn_table_count add_entry cheetah_topn_count cheetah_flowId 1 action_index 1
    pd cheetah_topn_t_one_table add_entry cheetah_topn_t_one_action cheetah_flowId 1 cheetah_md_topn_more_than_n 1 action_index 1
    pd cheetah_topn_t_one_table add_entry nop cheetah_flowId 1 cheetah_md_topn_more_than_n 0
    pd cheetah_topn_debug_table add_entry cheetah_topn_debug_one_action cheetah_flowId 1 action_index 1
    pd register_write cheetah_topn_min_reg_one index 1 register_value 10000


## Group By
    pd table_sketch_one_key add_entry action_sketch_one_key cheetah_flowId 1
    pd table_sketch_one_value add_entry action_sketch_one_value cheetah_flowId 1
    pd table_sketch_two_key add_entry action_sketch_two_key cheetah_flowId 1
    pd table_sketch_two_value add_entry action_sketch_two_value cheetah_flowId 1
    pd table_sketch_three_key add_entry action_sketch_three_key cheetah_flowId 1
    pd table_sketch_three_value add_entry action_sketch_three_value cheetah_flowId 1
    pd table_sketch_four_key add_entry action_sketch_four_key cheetah_flowId 1
    pd table_sketch_four_value add_entry action_sketch_four_value cheetah_flowId 1
    pd table_sketch_five_key add_entry action_sketch_five_key cheetah_flowId 1
    pd table_sketch_five_value add_entry action_sketch_five_value cheetah_flowId 1
    pd table_hash_init add_entry modify_hash cheetah_flowId 1
    pd table_hash_init_two add_entry modify_hash_two cheetah_flowId 1
    pd table_hash_init_three add_entry modify_hash_three cheetah_flowId 1
    pd table_hash_init_four add_entry modify_hash_four cheetah_flowId 1
    pd table_hash_init_five add_entry modify_hash_five cheetah_flowId 1
    pd table_prune add_entry prune cheetah_flowId 1


## Join
    pd tbl_prune add_entry prune udp_dstPort 4242
    pd tbl_bloom_1 add_entry act_bloom_1 cheetah_flowId 1
    pd tbl_bloom_1 add_entry act_bloom_1_r cheetah_flowId 2
    pd tbl_bloom_2 add_entry act_bloom_2 cheetah_flowId 1
    pd tbl_bloom_2 add_entry act_bloom_2_r cheetah_flowId 2
    pd tbl_bloom_3 add_entry act_bloom_3 cheetah_flowId 1
    pd tbl_bloom_3 add_entry act_bloom_3_r cheetah_flowId 1
    pd tbl_bloom_4 add_entry act_bloom_4 cheetah_flowId 1
    pd tbl_bloom_4 add_entry act_bloom_4_r cheetah_flowId 2
    pd tbl_bloom_5 add_entry act_bloom_5 cheetah_flowId 1
    pd tbl_bloom_5 add_entry act_bloom_5_r cheetah_flowId 2
    pd tbl_bloom_6 add_entry act_bloom_6 cheetah_flowId 1
    pd tbl_bloom_6 add_entry act_bloom_6 cheetah_flowId 2
    pd tbl_prepare_keys add_entry act_prepare_keys udp_dstPort 4242
    pd tbl_prepare_keys_2 add_entry act_prepare_keys_2 udp_dstPort 4242
