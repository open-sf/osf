print-%:
	@echo '$*=$($*)'

CONTIKI_PROJECT = node
all: $(CONTIKI_PROJECT)

#----------------------------------------------------------------------------#
# Contiki
#----------------------------------------------------------------------------#
WERROR = 0
CONTIKI = ../..
include $(CONTIKI)/Makefile.identify-target
include $(CONTIKI)/Makefile.dir-variables
MODULES_REL += $(TARGET)

# Enable app_cofig.h
CFLAGS += -DUSE_APP_CONFIG

# Contiki netstack
MAKE_MAC = MAKE_MAC_OSF
MAKE_ROUTING = MAKE_ROUTING_NULLROUTING
MAKE_NET = MAKE_NET_NULLNET

#----------------------------------------------------------------------------#
# HELLO WORLD
#----------------------------------------------------------------------------#
ifeq ($(HELLO_WORLD),1)
CFLAGS += -DHELLO_WORLD=1
endif

#----------------------------------------------------------------------------#
# Testbed/Deployment
#----------------------------------------------------------------------------#
DEPLOYMENT ?= nulltb
ifneq ($(DEPLOYMENT),)
  ifeq ($(DEPLOYMENT), nulltb)
  	CFLAGS += -DDEPLOYMENT_MAPPING=deployment_nulltb
  endif
  ifeq ($(DEPLOYMENT), dcube)
  	CFLAGS += -DDEPLOYMENT_MAPPING=deployment_dcube
  endif
  MODULES += $(CONTIKI_NG_SERVICES_DIR)/deployment
else
  $(warning WARN: No deployment specified! DEPLOYMENT=(nulltb/dcube))
endif

ifneq ($(TESTBED),)
    MODULES += $(CONTIKI_NG_SERVICES_DIR)/testbed
# else
#  $(warning WARN: No testbed specified! TESTBED=(nulltb/dcube))
endif

# NULLTB Configuration (ignored in D-Cube)
ifneq ($(SRC),)
    CFLAGS += -DTB_CONF_SOURCES=$(SRC)
endif
ifneq ($(DST),)
    CFLAGS += -DTB_CONF_DESTINATIONS=$(DST)
endif
ifneq ($(FWD),)
    CFLAGS += -DTB_CONF_FORWARDERS=$(FWD)
endif

#----------------------------------------------------------------------------#
# Base Configuration
#----------------------------------------------------------------------------#
ifneq ($(TS),)
    CFLAGS += -DTS=$(TS)
endif

ifneq ($(PROTO),)
    CFLAGS += -DPROTO=$(PROTO)
endif

ifneq ($(LENGTH),)
    CFLAGS += -DLENGTH=$(LENGTH)
endif

ifneq ($(PERIOD),)
    CFLAGS += -DPERIOD=$(PERIOD)
endif

ifneq ($(NTX),)
    CFLAGS += -DNTX=$(NTX)
endif

ifneq ($(PHY),)
    CFLAGS += -DPHY=$(PHY)
endif

ifneq ($(PRIMITIVE),)
    CFLAGS += -DPRIMITIVE=$(PRIMITIVE)
endif

ifneq ($(NSLOTS),)
    CFLAGS += -DNSLOTS=$(NSLOTS)
endif

ifneq ($(CHN),)
    CFLAGS += -DCHN=$(CHN)
endif

ifneq ($(PWR),)
    CFLAGS += -DPWR=$(PWR)
endif

ifneq ($(LOGGING),)
    CFLAGS += -DLOGGING=$(LOGGING)
		ifeq ($(LOGGING), 1)
    $(warning "LOGGING=1")
    endif
endif

ifneq ($(GPIO),)
    CFLAGS += -DGPIO=$(GPIO)
    ifeq ($(GPIO), 1)
    $(warning "GPIO=1")
    endif
endif

ifneq ($(LEDS),)
    CFLAGS += -DLEDS=$(LEDS)
    ifeq ($(LEDS), 1)
    $(warning "LEDS=1")
    endif
endif

#----------------------------------------------------------------------------#
# Protocol Configuration
#----------------------------------------------------------------------------#
ifneq ($(NTA),)
		CFLAGS += -DNTA=$(NTA)
endif
ifneq ($(EMPTY),)
    CFLAGS += -DEMPTY=$(EMPTY)
endif
ifneq ($(TOG),)
    CFLAGS += -DTOG=$(TOG)
endif
ifneq ($(ALWAYS_ACK),)
    CFLAGS += -DALWAYS_ACK=$(ALWAYS_ACK)
endif
ifneq ($(MPHY),)
    CFLAGS += -DMPHY=$(MPHY)
endif


#----------------------------------------------------------------------------#
# Protocol Extensions
#----------------------------------------------------------------------------#
ifneq ($(BACKOFF),)
    CFLAGS += -DBACKOFF=$(BACKOFF)
endif
ifneq ($(ND),)
    CFLAGS += -DND=$(ND)
endif

#----------------------------------------------------------------------------#
# Driver Extensions
#----------------------------------------------------------------------------#
ifneq ($(RNTX),)
    CFLAGS += -DRNTX=$(RNTX)
endif
ifneq ($(BV),)
    CFLAGS += -DBV=$(BV)
endif


#----------------------------------------------------------------------------#
# TEST Configuration
#----------------------------------------------------------------------------#
ifneq ($(DCUBE),)
    CFLAGS += -DDCUBE=$(DCUBE)
endif
ifneq ($(MISS_RXS),)
    CFLAGS += -DMISS_RXS=$(MISS_RXS)
endif
ifneq ($(TEST_NODE),)
    CFLAGS += -DTEST_NODE=$(TEST_NODE)
endif

#----------------------------------------------------------------------------#
# Serial over USB
##---------------------------------------------------------------------------#
ifneq ($(USB),)
NRF52840_NATIVE_USB=1
CFLAGS += -DNRF52840_NATIVE_USB=1
MODULES += $(CONTIKI_NG_SERVICES_DIR)/slip-cmd
# Extra modules for testing
ifeq ($(WITH_SHELL), 1)
	 $(warning "WITH_SHELL=1")
	 MODULES += $(CONTIKI_NG_SERVICES_DIR)/shell
endif
endif

include $(CONTIKI)/Makefile.include
