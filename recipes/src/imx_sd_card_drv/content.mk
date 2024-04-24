MIRROR_FROM_REP_DIR := src/drivers/sd_card \
                       src/include/spec/arm_v8a/lx_emul

MIRROR_FROM_OS_DIR  := src/lib/genode_c_api/block.cc

content: $(MIRROR_FROM_REP_DIR) $(MIRROR_FROM_OS_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

$(MIRROR_FROM_OS_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/os/$@ $@
