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
MAKE_ROUTING = MAKE_ROUTING_RPL_LITE
MAKE_NET = MAKE_NET_IPV6

#----------------------------------------------------------------------------#
# MAC
#----------------------------------------------------------------------------#
ifeq ($(MAC),TSCH)
  MAKE_MAC = MAKE_MAC_TSCH
# If Orchestra is NOT used then TSCH uses 6TiSCH minimal.
ifneq ($(ORCHESTRA),)
	MAKE_WITH_ORCHESTRA = 1
  MODULES += os/services/orchestra
endif
endif

#----------------------------------------------------------------------------#
# ROUTING
#----------------------------------------------------------------------------#
ifneq ($(RPL_CLASSIC),)
  MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC
  CFLAGS += -DRPL_CONF_WITH_NON_STORING=1
endif

ifneq ($(MCAST),)
	MODULES += $(CONTIKI_NG_NET_DIR)/ipv6/multicast
  CFLAGS += -DMULTICAST=1
endif

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
MODULES += $(CONTIKI_NG_SERVICES_DIR)/rpl-border-router
# Include webserver module
MODULES_REL += webserver
MODULES_REL += $(TARGET)
endif


#----------------------------------------------------------------------------#
include $(CONTIKI)/Makefile.include
