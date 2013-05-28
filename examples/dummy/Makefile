CONTIKI_PROJECT = dummy 
all: $(CONTIKI_PROJECT)

UIP_CONF_IPV6=1
#UIP_CONF_RPL=0
CFLAGS += -fPIC -g -lrt -lpthread
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
