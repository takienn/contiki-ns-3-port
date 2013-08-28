all: udp-client udp-server

UIP_CONF_IPV6=1
#UIP_CONF_RPL=0
CFLAGS += -fPIC -g -O3 -lrt -lpthread -ldl -rdynamic -DLOG
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
