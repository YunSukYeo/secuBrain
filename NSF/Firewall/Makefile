######################################################################
# Introduction example 6-c_config
# (C) 2006 Tail-f Systems
#
# See the README file for more information
######################################################################

usage:
	@echo "See README file for more instructions"
	@echo "make all     Build all example files"
	@echo "make clean   Remove all built and intermediary files"
	@echo "make start   Start CONFD daemon and example agent"
	@echo "make stop    Stop any CONFD daemon and example agent"
	@echo "make query   Run query against CONFD"
	@echo "make cli     Start the CONFD Command Line Interface, J-style"
	@echo "make cli-c   Start the CONFD Command Line Interface, C-style"



######################################################################
# Where is ConfD installed? Make sure CONFD_DIR points it out
CONFD_DIR ?= ../../../confd-6.2
INTERFACE_DIR ?= ../../Interfaces
BINARY_DIR ?= ../../bin
# Include standard ConfD build definitions and rules
include $(CONFD_DIR)/src/confd/build/include.mk

# In case CONFD_DIR is not set (correctly), this rule will trigger
$(CONFD_DIR)/src/confd/build/include.mk:
	@echo 'Where is ConfD installed? Set $$CONFD_DIR to point it out!'
	@echo ''

######################################################################
# Example specific definitions and rules

CONFD_FLAGS = --addloadpath $(CONFD_DIR)/etc/confd 
START_FLAGS ?=

######################################################################
# Example specific definitions and rules

CONFD_FLAGS ?=
START_FLAGS ?=

LIBS    += -lmysqlclient
SRC	= main.c nsf-secu-controller-interface.c $(INTERFACE_DIR)/nsf-sff-interface.c $(INTERFACE_DIR)/mysql-interface.c
OBJS	= nsf-secu-controller-interface.o nsf-sff-interface.o mysql-interface.o main.o #$(SRC:.c=.o)

#all:	firewall nsf-sff-interface hst.fxs hst.h hosts $(CDB_DIR) ssh-keydir firewall
all:	$(INTERFACE_DIR)/nsf-sff-interface.h nsf-sff-interface hst.fxs hst.h $(CDB_DIR) ssh-keydir $(INTERFACE_DIR)/mysql-interface.h mysql-interface nsf-secu-controller-interface.h nsf-secu-controller-interface.c main firewall 
	@echo $(OBJS)
	@echo "Build complete"

firewall: $(OBJS)
	$(CC) -o $(BINARY_DIR)/firewall $(OBJS) $(LIBS)

main:
	$(CC) -c main.c

mysql-interface: $(INTERFACE_DIR)/mysql-interface.h
	$(CC) -c $(INTERFACE_DIR)/mysql-interface.c
	@echo "mysql-Interface Build complete"

nsf-sff-interface: $(INTERFACE_DIR)/nsf-sff-interface.h $(INTERFACE_DIR)/constants.h
	$(CC) -c $(INTERFACE_DIR)/nsf-sff-interface.c
	@echo "NSF-SFF-Interface Build complete"

nsf-secu-controller-interface: ./nsf-secu-controller-interface.h $(INTERFACE_DIR)/mysql-interface.h
	$(CC) -c nsf-secu-controller-interface.c -Wall -g -I../../../confd-6.2/include
	@echo "nsf-secu-controller-Interface Build complete"


######################################################################
clean:	clean
	-rm -rf hst.h hosts RUNNING.ckp 2> /dev/null || true
	-rm -rf *.o
	-rm -rf $(BINARY_DIR)/firewall

init:  stop 
	$(CONFD)  -c confd.conf $(CONFD_FLAGS)
	### * In one terminal window, run: tail -f ./confd.log
	### * In another terminal window, run queries
	###   (try 'make query' for an example)
	### * In this window, the HOSTS confd daemon now starts:
	#../../bin/firewall $(START_FLAGS)

######################################################################
stop:
	### Killing any confd daemon and HOSTS confd agents
	$(CONFD) --stop || true
	$(KILLALL) hosts || true 
