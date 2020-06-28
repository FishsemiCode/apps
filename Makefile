############################################################################
# apps/Makefile
#
#   Copyright (C) 2011 Uros Platise. All rights reserved.
#   Copyright (C) 2011-2014, 2018-2019 Gregory Nutt. All rights reserved.
#   Authors: Uros Platise <uros.platise@isotel.eu>
#            Gregory Nutt <gnutt@nuttx.org>
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

APPDIR := $(patsubst %/,%,$(dir $(firstword $(MAKEFILE_LIST))))
TOPDIR ?= $(APPDIR)/import
OUTDIRAPPS := $(OUTDIR)/../apps
-include $(TOPDIR)/Make.defs
-include $(APPDIR)/Make.defs

# Application Directories

# BUILDIRS is the list of top-level directories containing Make.defs files
# CLEANDIRS is the list of all top-level directories containing Makefiles.
#   It is used only for cleaning.

BUILDIRS   := $(dir $(filter-out $(APPDIR)/import/Make.defs,$(wildcard $(APPDIR)/*/Make.defs)))
CLEANDIRS  := $(dir $(wildcard $(APPDIR)/*/Makefile))

BUILDIRS := $(patsubst $(APPDIR)/%/,%,$(BUILDIRS))
CLEANDIRS := $(patsubst $(APPDIR)/%/,%,$(CLEANDIRS))

# CONFIGURED_APPS is the application directories that should be built in
#   the current configuration.

CONFIGURED_APPS =

define Add_Application
  include $(1)/Make.defs
endef

$(foreach BDIR, $(BUILDIRS), $(eval $(call Add_Application,$(BDIR))))

# Library path

LIBPATH ?= $(TOPDIR)$(DELIM)staging

# The final build target

ifeq ($(CONFIG_WINDOWS_NATIVE),y)
APPDIR := ${shell echo %CD%}
endif

BIN = libapps$(LIBEXT)
VPATH := $(APPDIR)

# Symbol table for loadable apps.

EXETABSRC = $(SYM_DIR)$(DELIM)symtab_apps.c
MODTABSRC = $(SYM_DIR)$(DELIM)modtab_apps.c
SYMTABSRC = $(EXETABSRC) $(MODTABSRC)
SYMTABOBJ = $(SYMTABSRC:.c=$(OBJEXT))

# Build targets

all: $(BIN)
.PHONY: import install dirlinks context context_serialize clean_context context_rest .depdirs preconfig depend clean distclean all_target
.PRECIOUS: libapps$(LIBEXT)

define MAKE_template
	$(call MKDIR, $(1))
	$(Q) $(MAKE) -C $(1) -f $(APPDIR)/$(1)/Makefile -I $(APPDIR)/$(1) $(2) TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)"

endef

define SDIR_template
$(1)_$(2):
	$(call MKDIR, $(1))
	$(Q) $(MAKE) -C $(subst $(APPDIR)/,,$(1)) -f $(1)/Makefile -I $(1) $(2) TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)"

endef

$(foreach SDIR, $(CONFIGURED_APPS), $(eval $(call SDIR_template,$(SDIR),all)))
$(foreach SDIR, $(CONFIGURED_APPS), $(eval $(call SDIR_template,$(SDIR),install)))
$(foreach SDIR, $(CONFIGURED_APPS), $(eval $(call SDIR_template,$(SDIR),context)))
$(foreach SDIR, $(CONFIGURED_APPS), $(eval $(call SDIR_template,$(SDIR),depend)))
$(foreach SDIR, $(CLEANDIRS), $(eval $(call SDIR_template,$(SDIR),clean)))
$(foreach SDIR, $(CLEANDIRS), $(eval $(call SDIR_template,$(SDIR),distclean)))

# In the KERNEL build, we must build and install all of the modules.  No
# symbol table is needed

MKDIR_LIST = $(BINSYM_DIR) $(LIBSYM_DIR) $(BINIST_DIR) $(LIBIST_DIR)

ifeq ($(CONFIG_BUILD_KERNEL),y)

.install: $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_install)

install: $(MKDIR_LIST) .install

$(MKDIR_LIST):
	$(Q) mkdir -p $@

.import: $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_all)
	$(Q) $(MAKE) -C $(OUTDIRAPPS) -f $(APPDIR)/Makefile -I $(APPDIR) install TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" OUTDIR="$(OUTDIR)"

import: $(MKDIR_LIST)
	$(Q) $(MAKE) -C $(OUTDIRAPPS) -f $(APPDIR)/Makefile -I $(APPDIR) .import TOPDIR="$(APPDIR)/import" APPDIR="$(APPDIR)" OUTDIR="$(OUTDIR)"

else

# In FLAT and protected modes, the modules have already been created.  A
# symbol table is required.

ifeq ($(CONFIG_BUILD_LOADABLE),)

$(BIN): $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_all)

else

all_target: $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_all)
	$(Q) $(MAKE) -C $(OUTDIRAPPS) -f $(APPDIR)/Makefile -I $(APPDIR) install TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)" OUTDIR="$(OUTDIR)"

$(EXETABSRC): all_target
	$(Q) $(APPDIR)$(DELIM)tools$(DELIM)mksymtab.sh $(BINIST_DIR) $@

$(MODTABSRC): all_target
	$(Q) $(APPDIR)$(DELIM)tools$(DELIM)mkmodsymtab.sh $(LIBIST_DIR) $@

$(SYMTABOBJ): %$(OBJEXT): %.c
	$(call COMPILE, -fno-lto $<, $@)

$(BIN): $(SYMTABOBJ)
ifeq ($(WINTOOL),y)
	$(call ARCHIVE, "${shell cygpath -w $(BIN)}", $^)
else
	$(call ARCHIVE, $(BIN), $^)
endif

endif # !CONFIG_BUILD_LOADABLE

.install: $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_install)

$(MKDIR_LIST):
	$(Q) mkdir -p $@

install: $(MKDIR_LIST) .install

.import: $(BIN) install

import:
	$(Q) $(MAKE) -C $(OUTDIRAPPS) -f $(APPDIR)/Makefile -I $(APPDIR) .import TOPDIR="$(APPDIR)/import" APPDIR="$(APPDIR)" OUTDIR="$(OUTDIR)"

endif # CONFIG_BUILD_KERNEL

dirlinks:
	$(call MKDIR, platform)
	$(Q) $(MAKE) -C platform -f $(APPDIR)/platform/Makefile -I $(APPDIR)/platform dirlinks TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)"

context_rest: $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_context)

context_serialize:
	$(Q) $(MAKE) -C builtin -f $(APPDIR)/builtin/Makefile -I $(APPDIR)/builtin context TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)"
	$(Q) $(MAKE) -f $(APPDIR)/Makefile -I $(APPDIR) context_rest

context: context_serialize

Kconfig:
	$(foreach SDIR, $(BUILDIRS), $(call MAKE_template,$(SDIR),preconfig))
	$(Q) $(MKKCONFIG) -i $(APPDIR) -o Kconfig

preconfig: Kconfig

export:
ifneq ($(EXPORTDIR),)
ifneq ($(BUILTIN_REGISTRY),)
	$(Q) mkdir -p "${EXPORTDIR}"/registry || exit 1; \
	for f in "${BUILTIN_REGISTRY}"/*.bdat "${BUILTIN_REGISTRY}"/*.pdat ; do \
		[ -f "$${f}" ] && cp -f "$${f}" "${EXPORTDIR}"/registry ; \
	done
endif
endif

.depdirs: $(foreach SDIR, $(CONFIGURED_APPS), $(SDIR)_depend)

.depend: Makefile .depdirs
	$(Q) touch $@

depend: .depend

clean_context:
	$(Q) $(MAKE) -C platform -f $(APPDIR)/platform/Makefile -I $(APPDIR)/platform clean_context TOPDIR="$(TOPDIR)" APPDIR="$(APPDIR)"

clean: $(foreach SDIR, $(CLEANDIRS), $(SDIR)_clean)
	$(call DELFILE, $(SYMTABSRC))
	$(call DELFILE, $(SYMTABOBJ))
	$(call DELFILE, $(BIN))
	$(call DELFILE, Kconfig)
	$(call DELDIR, $(SYM_DIR))
	$(call DELDIR, $(IST_DIR))
	$(call DELDIR, $(BINDIR))
	$(call CLEAN)

distclean: $(foreach SDIR, $(CLEANDIRS), $(SDIR)_distclean)
ifeq ($(CONFIG_WINDOWS_NATIVE),y)
	$(Q) ( if exist  external ( \
		echo ********************************************************" \
		echo * The external directory/link must be removed manually *" \
		echo ********************************************************" \
	)
else
	$(Q) ( if [ -e external ]; then \
		echo "********************************************************"; \
		echo "* The external directory/link must be removed manually *"; \
		echo "********************************************************"; \
		fi; \
	)
endif
	$(call DELFILE, *.lock)
	$(call DELFILE, .depend)
	$(call DELFILE, $(SYMTABSRC))
	$(call DELFILE, $(SYMTABOBJ))
	$(call DELFILE, $(BIN))
	$(call DELFILE, Kconfig)
	$(call DELDIR, $(SYM_DIR))
	$(call DELDIR, $(IST_DIR))
	$(call DELDIR, $(BINDIR))
	$(call CLEAN)
