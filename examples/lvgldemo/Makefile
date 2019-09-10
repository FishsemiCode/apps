############################################################################
# apps/examples/lgvldemo/Makefile
#
#   Copyright (C) 2018 Gregory Nutt. All rights reserved.
#   Author: Gregory Nutt <gnutt@nuttx.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name NuttX nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

-include $(TOPDIR)/Make.defs

# LittleVGL demo built-in application info

CONFIG_EXAMPLES_LVGLDEMO_PRIORITY ?= SCHED_PRIORITY_DEFAULT
CONFIG_EXAMPLES_LVGLDEMO_STACKSIZE ?= 32768

APPNAME = lvgldemo
PRIORITY = $(CONFIG_EXAMPLES_LVGLDEMO_PRIORITY)
STACKSIZE = $(CONFIG_EXAMPLES_LVGLDEMO_STACKSIZE)

# LittleVGL demo Example

ASRCS =

LVGL_EXAMPLE_DIR = $(APPDIR)/examples/lvgldemo
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_apps/demo/demo.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_group/lv_test_group.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_theme/lv_test_theme.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_obj/lv_test_obj.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_stress/lv_test_stress.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_theme/lv_test_theme.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_group/lv_test_group.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_arc/lv_test_arc.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_bar/lv_test_bar.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_btn/lv_test_btn.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_btnm/lv_test_btnm.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_cb/lv_test_cb.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_chart/lv_test_chart.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_canvas/lv_test_canvas.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_cont/lv_test_cont.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_ddlist/lv_test_ddlist.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_gauge/lv_test_gauge.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_img/lv_test_img.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_imgbtn/lv_test_imgbtn.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_kb/lv_test_kb.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_label/lv_test_label.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_led/lv_test_led.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_line/lv_test_line.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_list/lv_test_list.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_lmeter/lv_test_lmeter.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_mbox/lv_test_mbox.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_page/lv_test_page.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_preload/lv_test_preload.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_roller/lv_test_roller.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_slider/lv_test_slider.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_sw/lv_test_sw.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_ta/lv_test_ta.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_tabview/lv_test_tabview.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_table/lv_test_table.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_tileview/lv_test_tileview.mk
include $(LVGL_EXAMPLE_DIR)/lv_examples/lv_tests/lv_test_objx/lv_test_win/lv_test_win.mk
CSRCS += fbdev.c
MAINSRC = lvgldemo.c


CONFIG_EXAMPLES_LVGLDEMO_PROGNAME ?= lvgldemo$(EXEEXT)
PROGNAME = $(CONFIG_EXAMPLES_LVGLDEMO_PROGNAME)

LVGLDIR=$(APPDIR)/graphics/littlevgl/lvgl
CFLAGS   += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "$(LVGLDIR)"}
CXXFLAGS += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "$(LVGLDIR)"}
CFLAGS   += -DLV_CONF_INCLUDE_SIMPLE_EXAMPLE
CFLAGS   += -I$(LVGL_EXAMPLE_DIR)

MODULE = CONFIG_EXAMPLES_LVGLDEMO

include $(APPDIR)/Application.mk
