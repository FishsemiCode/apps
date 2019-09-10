CSRCS += lv_test_group.c

DEPPATH += --dep-path $(LVGL_DIR)/lv_examples/lv_tests/lv_test_group
VPATH += :$(LVGL_DIR)/lv_examples/lv_tests/lv_test_group

CFLAGS += "-I$(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_group"
