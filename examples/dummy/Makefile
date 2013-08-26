CONTIKI_PROJECT = dummy 
all: $(CONTIKI_PROJECT)

UIP_CONF_IPV6=1
UIP_CONF_RPL=0
CFLAGS += -fPIC -g -O3 -lrt -lpthread -ldl -rdynamic -DLOG
CFLAGS += -DUIP_CONF_IPV6=1
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
