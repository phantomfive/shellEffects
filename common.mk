# To be included in other makefiles, containing common definitions
# and common make patterns

#Instructions:
#
# To use, first define the following variables. Then add the line
# 'include common.mk' in your makefile.
#
# Variables to set when you include this file:
# OBJDIR - (required) The place to output .o files.
# INC    - if you have special include locations, set this to 
#           something like '-Idir -Idir2'
# LIBNAME - if it is set, libraries will be created from the OBJDIR source
# TOCLEAN - if you have any extra files you'd like removed as part of the
#           clean step, you can set them to this variable, space separated

# From there, 'make objs' will cause all .c files to be built and stored in
# $(OUTDIR), 'make depend' will cause the dependencies to be stuck on the
# end of your makefile, and 'make clean' will remove $(OBJDIR) and $(TOCLEAN).
# If you want other targets, you'll have to write them yourself.
#

CC=gcc
CFLAGS=-Wall -Werror -std=c99  -ggdb
LDFLAGS=-lm

CSRC := $(wildcard *.c)
OBJS = $(patsubst %.c,$(OBJDIR)/%.o, $(CSRC))

defaultTarget: objs $(LIBNAME).a


#target for making objects
objs: $(OBJDIR) $(OBJS) 
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@
$(OBJDIR): 
	mkdir -p $(OBJDIR)

#pattern for making static libraries
$(LIBNAME).a: $(OBJS)
ifdef LIBNAME
	mkdir -p $(dir $(LIBNAME))
	ar rcs $(LIBNAME).a $(OBJS)
endif


depend: $(CSRC)
	makedepend -Y -p$(OBJDIR) $(INC) $(CSRC) 2> /dev/null
	rm Makefile.bak

clean:
	rm -rf $(OBJDIR) $(TOCLEAN)

.PHONY: objs depend defaultTarget clean

