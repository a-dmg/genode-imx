MIRROR_FROM_REP_DIR  := src/drivers/watchdog

content: LICENSE $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

LICENSE:
	cp $(GENODE_DIR)/LICENSE $@
