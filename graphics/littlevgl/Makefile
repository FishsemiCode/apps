############################################################################
# apps/graphics/littlevgl/Makefile
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

# LVGL graphic library

LVGL_DIR = .

# LVGL Libraries

CURR_DIR = $(APPDIR)/graphics/littlevgl

-include $(CURR_DIR)/lvgl/src/lv_core/lv_core.mk
-include $(CURR_DIR)/lvgl/src/lv_hal/lv_hal.mk
-include $(CURR_DIR)/lvgl/src/lv_objx/lv_objx.mk
-include $(CURR_DIR)/lvgl/src/lv_font/lv_font.mk
-include $(CURR_DIR)/lvgl/src/lv_misc/lv_misc.mk
-include $(CURR_DIR)/lvgl/src/lv_themes/lv_themes.mk
-include $(CURR_DIR)/lvgl/src/lv_draw/lv_draw.mk

# Set up build configuration and environment

WD := ${shell echo $(CURDIR) | sed -e 's/ /\\ /g'}

CONFIG_GRAPH_LVGL_URL ?= "https://github.com/littlevgl/lvgl/archive"

LVGL_VERSION ?= 6.1.2
LVGL_TARBALL = v$(LVGL_VERSION).zip
WGET ?= wget

LVGL_UNPACKNAME = lvgl
UNPACK ?= unzip -o

PATCH_FILES = lv_fix_warning.patch
PATCH ?= patch -p0

LVGL_UNPACKDIR =  $(WD)/$(LVGL_UNPACKNAME)

CFLAGS += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" $(APPDIR)/graphics/littlevgl}

$(LVGL_TARBALL):
	@echo "Downloading: $(LVGL_TARBALL)"
	$(Q) $(WGET) $(CONFIG_GRAPH_LVGL_URL)/$(LVGL_TARBALL)

$(LVGL_UNPACKNAME): $(LVGL_TARBALL)
	@echo "Unpacking: $(LVGL_TARBALL) -> $(LVGL_UNPACKNAME)"
	$(Q) $(UNPACK) $(LVGL_TARBALL)
	$(Q) mv	lvgl-$(LVGL_VERSION) $(LVGL_UNPACKNAME)
	$(Q) touch $(LVGL_UNPACKNAME)

$(LVGL_UNPACKNAME)/.patched: $(LVGL_UNPACKNAME) $(PATCH_FILES)
	$(Q) cat $(PATCH_FILES) | $(PATCH)
	$(Q) touch $(LVGL_UNPACKNAME)/.patched

lvgl/lvgl.h: $(LVGL_UNPACKNAME)

$(APPDIR)/include/graphics/lvgl.h: lvgl/lvgl.h $(LVGL_UNPACKNAME)/.patched
	@echo "CP: lvgl/lvgl.h"
	$(Q) cp lvgl/lvgl.h $(APPDIR)/include/graphics/lvgl.h

#context:: $(LVGL_UNPACKNAME) $(APPDIR)/include/graphics/lvgl.h

clean::
	$(call DELDIR, build)

#distclean::
#	$(call DELDIR, $(LVGL_UNPACKNAME))
#	$(call DELFILE, $(LVGL_TARBALL))
#	$(call DELFILE, $(APPDIR)/include/graphics/lvgl.h)

include $(APPDIR)/Application.mk
