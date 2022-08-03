CONTIKI_PROJECT = node
all: $(CONTIKI_PROJECT)

# D-Cube only supports the nrf52840-DK and sky
PLATFORMS_ONLY= nrf nrf52840 sky
# The BR is either native or embedded, and in the latter case must support SLIP
BOARDS_EXCLUDE = nrf52840/dongle dongle
# Enable app_cofig.h
CFLAGS += -DUSE_APP_CONFIG

#----------------------------------------------------------------------------#
# Contiki
#----------------------------------------------------------------------------#
WERROR = 0
CONTIKI = ../..
include $(CONTIKI)/Makefile.identify-target

# SMALL ?= 1
MAKE_MAC ?= MAKE_MAC_CSMA
MAKE_ROUTING = MAKE_ROUTING_NULLROUTING
MAKE_NET = MAKE_NET_IPV6

#----------------------------------------------------------------------------#
# Testbed/Deployment
#----------------------------------------------------------------------------#
ifneq ($(DEPLOYMENT),)
  ifeq ($(DEPLOYMENT), nulltb)
  	CFLAGS += -DDEPLOYMENT_MAPPING=deployment_nulltb
  endif
  ifeq ($(DEPLOYMENT), dcube)
  	CFLAGS += -DDEPLOYMENT_MAPPING=deployment_dcube
  endif
  MODULES += os/services/deployment
# else
#  $(warning WARN: No deployment specified! DEPLOYMENT=(nulltb/dcube))
endif

ifneq ($(TESTBED),)
    MODULES += os/services/testbed
# else
#  $(warning WARN: No testbed specified! TESTBED=(nulltb/dcube))
endif

# NULLTB Configuration (ignored in D-Cube)
ifneq ($(PATTERN),)
    CFLAGS += -DTB_CONF_PATTERN=$(PATTERN)
endif
ifneq ($(SRC),)
    CFLAGS += -DTB_CONF_SOURCES=$(SRC)
endif
ifneq ($(DST),)
    CFLAGS += -DTB_CONF_DESTINATIONS=$(DST)
endif
ifneq ($(FWD),)
    CFLAGS += -DTB_CONF_FORWARDERS=$(FWD)
endif
ifneq ($(BR),)
    CFLAGS += -DTB_CONF_BRS=$(BR)
endif

#----------------------------------------------------------------------------#
# Border Router
#----------------------------------------------------------------------------#
ifneq ($(WITH_BORDER_ROUTER),)
NRF52840_NATIVE_USB = 1
CFLAGS += -DTESTBED_WITH_BORDER_ROUTER
# Include RPL BR module
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_SERVICES_DIR)/null-border-router
# Include webserver module
MODULES_REL += webserver
MODULES_REL += $(TARGET)
endif

#----------------------------------------------------------------------------#
# Contiki
#----------------------------------------------------------------------------#
include $(CONTIKI)/Makefile.include
