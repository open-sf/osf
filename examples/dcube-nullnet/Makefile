CONTIKI_PROJECT = node
all: $(CONTIKI_PROJECT)

#----------------------------------------------------------------------------#
# Contiki
#----------------------------------------------------------------------------#
WERROR = 0
CONTIKI = ../..
include $(CONTIKI)/Makefile.identify-target

# SMALL ?= 1
MAKE_MAC ?= MAKE_MAC_CSMA
MAKE_ROUTING = MAKE_ROUTING_NULLROUTING
MAKE_NET = MAKE_NET_NULLNET

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
# Contiki
#----------------------------------------------------------------------------#
include $(CONTIKI)/Makefile.include
