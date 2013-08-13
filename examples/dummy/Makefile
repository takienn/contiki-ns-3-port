CONTIKI_PROJECT = dummy 
all: $(CONTIKI_PROJECT)

UIP_CONF_IPV6=1
#UIP_CONF_RPL=0
CFLAGS += -fPIC -g -O0 -lrt -lpthread -ldl -rdynamic -DLOG
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
