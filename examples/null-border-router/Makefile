CONTIKI_PROJECT = node
all: $(CONTIKI_PROJECT)

# Has only been tested on nrf52840
PLATFORMS_ONLY= nrf nrf52840
# The BR is either native or embedded, and in the latter case must support SLIP
BOARDS_EXCLUDE = nrf52840/dongle dongle
# Enable app_cofig.h
CFLAGS += -DUSE_APP_CONFIG

#----------------------------------------------------------------------------#
# Contiki
#----------------------------------------------------------------------------#
CONTIKI = ../..
include $(CONTIKI)/Makefile.identify-target

# SMALL ?= 1
MAKE_MAC ?= MAKE_MAC_CSMA
MAKE_ROUTING = MAKE_ROUTING_NULLROUTING
MAKE_NET = MAKE_NET_IPV6

#----------------------------------------------------------------------------#
# Border Router
#----------------------------------------------------------------------------#
NRF52840_NATIVE_USB = 1
CFLAGS += -DTESTBED_WITH_BORDER_ROUTER
# Include RPL BR module
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_SERVICES_DIR)/null-border-router
# Include webserver module
MODULES_REL += webserver
MODULES_REL += $(TARGET)

#----------------------------------------------------------------------------#
# Contiki
#----------------------------------------------------------------------------#
include $(CONTIKI)/Makefile.include
