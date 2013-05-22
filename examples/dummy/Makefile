CONTIKI_PROJECT = dummy 
all: $(CONTIKI_PROJECT)

UIP_CONF_IPV6=1
CFLAGS += -fPIC -g -lrt -lpthread -DDEBUG=0
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
