# (Not so) simple make file

#  Copyright (c) 2017 Raymond S Brand
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#
#   * Redistributions in source or binary form must carry prominent
#     notices of any modifications.
#
#   * Neither the name of the Raymond S Brand nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
#  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.


VERSION_API	= 1
VERSION_FEATURE	= 0
VERSION_PATCH	= 0
VERSION_LOCAL	= 0
VERSION_BUILD	= 0
VERSION_SPECIAL	= Flying Moose



CMDS		:= test_ExtArr
CMD_SRCS	:= $(CMDS:%=%.c)

LIBEXTARR	:= libExtArr.so
LIBEXTARR_NAME	:= $(LIBEXTARR).$(VERSION_API).$(VERSION_FEATURE)
LIBEXTARR_FILE	:= $(LIBEXTARR_NAME).$(VERSION_PATCH).$(VERSION_LOCAL).$(VERSION_BUILD)
EXTARR_SRCS	:= ExtArr.c
EXTARR_OBJS	:= $(EXTARR_SRCS:%.c=%.o)


ALL_SRCS	:= $(CMD_SRCS) $(EXTARR_SRCS)
ALL_OBJS	:= $(ALL_SRCS:%.c=%.o)

TARGETS		:= $(CMDS) $(LIBEXTARR) $(LIBEXTARR_NAME) $(LIBEXTARR_FILE)


all:	$(TARGETS)

CC	= colorgcc
LD	= ld
LN	= ln -s
RM	= rm -f

CFLAGS	+= -O3 -Wall
CFLAGS	+= -std=c99
CFLAGS	+= -pedantic
CFLAGS	+= -Wextra -Wunused -Wuninitialized -Wundef -Wshadow -Wconversion
CFLAGS	+= -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations

$(EXTARR_OBJS):CFLAGS += -fpic
$(EXTARR_OBJS):CFLAGS += -DEXTARR_VERSION_API=$(VERSION_API)
$(EXTARR_OBJS):CFLAGS += -DEXTARR_VERSION_FEATURE=$(VERSION_FEATURE)
$(EXTARR_OBJS):CFLAGS += -DEXTARR_VERSION_PATCH=$(VERSION_PATCH)
$(EXTARR_OBJS):CFLAGS += -DEXTARR_VERSION_LOCAL=$(VERSION_LOCAL)
$(EXTARR_OBJS):CFLAGS += -DEXTARR_VERSION_BUILD=$(VERSION_BUILD)
$(EXTARR_OBJS):CFLAGS += "-DEXTARR_VERSION_SPECIAL=\"$(VERSION_SPECIAL)\""


test_ExtArr:	test_ExtArr.o $(LIBEXTARR) $(LIBEXTARR_NAME)
	@echo "\$$(CC) $@"
	@$(CC) $(LDFLAGS) $< -L . -lExtArr $(LDLIBS) -o $@


$(LIBEXTARR):	$(LIBEXTARR_FILE)
	@echo "\$$(LN) $@"
	@$(RM) $@
	@$(LN) $(LIBEXTARR_FILE) $@

$(LIBEXTARR_NAME):	$(LIBEXTARR_FILE)
	@echo "\$$(LN) $@"
	@$(RM) $@
	@$(LN) $(LIBEXTARR_FILE) $@

$(LIBEXTARR_FILE):	$(EXTARR_OBJS)
	@echo "\$$(LD) $@"
	@$(RM) $(LIBEXTARR_NAME) $(LIBEXTARR)
	@$(LD) -shared -soname $(LIBEXTARR_NAME) -o $(LIBEXTARR_FILE) -lc $^


DEPS	:= $(ALL_SRCS:%.c=.%.d)

.%.d: %.c Makefile
	@echo "\$$(CC) -MM $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(INCS) -MM -MF $@ -MQ $(<:%.c=%.o) -MQ '$@' $<

%.o: %.c .%.d
	@echo "\$$(CC) $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(INCS) -o $@ -c $<


clean::
	@$(RM) $(TARGETS) $(ALL_OBJS) $(DEPS)

symbols::	$(LIBEXTARR_FILE)
	@nm -D $(LIBEXTARR_FILE)

symbols_all::	$(LIBEXTARR_FILE)
	@readelf -Ws $(LIBEXTARR_FILE)

-include $(DEPS)
