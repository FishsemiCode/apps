############################################################################
# apps/ interpreters/micropython/Makefile
#
#   Copyright (C) 2015, 2016 Gregory Nutt. All rights reserved.
#   Authors: Gregory Nutt <gnutt@nuttx.org>
#            Dave Marples <dave@marples.net>
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
###########################################################################

-include $(TOPDIR)/.config

ifeq ($(CONFIG_INTERPRETERS_MICROPYTHON),y)
MICROPY_PORT_DIR = micropython/ports/nuttx

CREATEDIR = $(MICROPY_PORT_DIR)

-include $(TOPDIR)/Make.defs
include $(APPDIR)/Make.defs

.PHONY = $(MAKECMDGOALS) sub-make

$(MAKECMDGOALS): sub-make

sub-make:
	$(Q)$(MAKE) -C $(MICROPY_PORT_DIR) -f $(SRCDIR)/$(MICROPY_PORT_DIR)/Makefile -I $(SRCDIR)/$(MICROPY_PORT_DIR) TOPDIR=$(TOPDIR) APPDIR=$(APPDIR) OUTDIR=$(OUTDIR) $(MAKECMDGOALS)
else
include $(APPDIR)/Application.mk
endif
