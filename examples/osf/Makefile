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

# Enable app_config.h
CFLAGS += -DUSE_APP_CONFIG

# Contiki netstack
MAKE_MAC = MAKE_MAC_OSF
MAKE_ROUTING = MAKE_ROUTING_NULLROUTING
MAKE_NET = MAKE_NET_NULLNET
NETSTACK_PA = MAKE_PA_NULLPA

#----------------------------------------------------------------------------#
# Testbed/Deployment
#----------------------------------------------------------------------------#
DEPLOYMENT ?= nulltb
ifneq ($(DEPLOYMENT),)
  ifeq ($(DEPLOYMENT), nulltb)
  	CFLAGS += -DDEPLOYMENT_MAPPING=deployment_nulltb
    $(warning "DEPLOYMENT=nulltb")
  endif
  ifeq ($(DEPLOYMENT), dcube)
  	CFLAGS += -DDEPLOYMENT_MAPPING=deployment_dcube
    $(warning "DEPLOYMENT=dcube")
  endif
  MODULES += $(CONTIKI_NG_SERVICES_DIR)/deployment
else
  $(warning WARN: No deployment specified! DEPLOYMENT=(nulltb/dcube))
endif

ifneq ($(TESTBED),)
    MODULES += $(CONTIKI_NG_SERVICES_DIR)/testbed
    CFLAGS += -DTB_CONF_RX_FIFO_LEN=16 -DTB_CONF_TX_FIFO_LEN=16
    $(warning "TESTBED=1")
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
    # WARNING : when setting LENGTH, take HDR, MIC and PHY values in account !
    CFLAGS += -DLENGTH=$(LENGTH)
    $(warning "LENGTH=$(LENGTH) bytes")
endif
ifneq ($(PERIOD),)
    CFLAGS += -DPERIOD=$(PERIOD)
else
    CFLAGS += -DPERIOD=1 # automatic timing of an epoch    
endif
ifneq ($(NTX),)
    CFLAGS += -DNTX=$(NTX)
endif
ifneq ($(PHY),)
    CFLAGS += -DPHY=$(PHY)
    $(warning "PHY=$(PHY)")
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
ifneq ($(OSF_CHN),)
    CFLAGS += -DOSF_CHN=$(OSF_CHN)
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
    PROJECT_SOURCEFILES += node_leds.c
    $(warning "LEDS=1")
    endif
endif
ifneq ($(NDUART),)
    ifeq ($(NDUART), 1)
    CFLAGS += -DUART0_ENABLED=0  -DLOG_CONF_OUTPUT="(void)(0);" -Wno-unused-value
    $(warning "UART0_ENABLED=0")
    endif
endif

#----------------------------------------------------------------------------#
# Protocol Configuration
#----------------------------------------------------------------------------#
ifneq ($(NTA),)
    CFLAGS += -DNTA=$(NTA)
endif
ifneq ($(EMPTY),)
    CFLAGS += -DEMPTY=$(EMPTY) # TODO
endif
ifneq ($(RTX),)
    CFLAGS += -DRTX=$(RTX)
endif
#----------------------------------------------------------------------------#
# TEST Configuration
#----------------------------------------------------------------------------#
ifneq ($(MISS_RXS),)
    CFLAGS += -DMISS_RXS=$(MISS_RXS)
endif
ifneq ($(TEST_NODE),)
    CFLAGS += -DTEST_NODE=$(TEST_NODE)
endif

#----------------------------------------------------------------------------#
# IPV6
##---------------------------------------------------------------------------#
ifeq ($(IPV6), 1)
 ifneq ($(PREFIX),)
    CFLAGS += -DOSF_CONF_PREFIX=$(PREFIX)
    $(warning "PREFIX=$(PREFIX)")
 endif
 ifneq ($(ROUTES),)
    CFLAGS += -DOSF_CONF_ROUTES=$(ROUTES)
    $(warning "ROUTES=$(ROUTES)")
 endif
 ifeq ($(WITH_UDPSERVER), 1)
    $(warning "WITH_UDPSERVER=1")
    PROJECT_SOURCEFILES += udp-server.c
    CFLAGS += -DUSE_UDP_SERVER=1
    ifeq ($(WITH_UDPCLIENT), 1)
    $(warning "WITH_UDPCLIENT=1")
    PROJECT_SOURCEFILES += udp-client.c
    CFLAGS += -DUSE_UDP_CLIENT=1
    ifneq ($(DL),) # payload length
    CFLAGS += -DDL=$(DL)
    endif
    ifneq ($(DS),) # destination server node_id
    CFLAGS += -DDS=$(DS)
    endif
    endif
 endif
 ifeq ($(WITH_WEBSERVER), 1)
    $(warning "WITH_WEBSERVER=1")
    PROJECT_SOURCEFILES += webserver.c httpd-simple.c
    CFLAGS += -DUSE_WEB_SERVER=1 
    CFLAGS += -DWEB_SERVER_STAT=1 # Add SF and uIP transfer statistics
 endif
 ifeq ($(WITH_TCPSERVER), 1)
    $(warning "WITH_TCPSERVER=1")
    PROJECT_SOURCEFILES += tcp_server.c
    CFLAGS += -DUSE_TCP_SERVER=1
 endif
 ifeq ($(WITH_UDPCONTROL), 1)
    $(warning "WITH_UDPCONTROL=1")
    PROJECT_SOURCEFILES += udp-control.c
    CFLAGS += -DUSE_UDP_CONTROL=1 -DUDP_CONTROL_PORT=7777
 endif

 # Payload size ( + MIC ) for main PHY.
 # Custom header HDR overhead is 5 bytes ( phy_pkt_hdr + id/epoch)
 # MIC code overhead 4 bytes
 # (HDR+LENGTH) <= 255 ( IEEE 125) bytes
 # LENGTH = (PAYLOAD + MIC)
 ifneq ($(LENGTH),)
    # LENGTH of data specified, use it for S,T & A rounds if S&A are not shrinked
    # OSF_CONF_MAXLEN - max possible length of packet
    CFLAGS += -DOSF_CONF_MAXLEN=$(LENGTH)+5 -DLENGTH=$(LENGTH)
    CFLAGS += -UPACKETBUF_CONF_SIZE -DPACKETBUF_CONF_SIZE=$(LENGTH)-4
    # Length of S and A rounds are defined in project-conf.h
    CFLAGS += -DOSF_SHRINK_S_A_ROUNDS=1 # optimized S&A rounds
 else
    # LENGTH is not specified, set optimized values
    ifeq ($(PHY), PHY_IEEE)
    # PHY IEEE
    	LENGTH=120
    	CFLAGS += -DLENGTH=$(LENGTH)
    	CFLAGS += -UPACKETBUF_CONF_SIZE -DPACKETBUF_CONF_SIZE=$(LENGTH)
    else
    # PHY BLE
			LENGTH=250
    	CFLAGS += -DLENGTH=$(LENGTH)
    	CFLAGS += -UPACKETBUF_CONF_SIZE -DPACKETBUF_CONF_SIZE=$(LENGTH)-2 # 4byte alignment
    # Length of S and A rounds are defined in project-conf.h
    	CFLAGS += -DOSF_SHRINK_S_A_ROUNDS=1 # optimized S&A rounds
    endif # PHY
    $(warning "set LENGTH=$(LENGTH) bytes")
 endif
 
 MAKE_NET = MAKE_NET_IPV6
 CFLAGS += -DOSF_WITH_IPV6=1
 $(warning "NET : IPV6")

endif #IPV6

#----------------------------------------------------------------------------#
# Border Router
##---------------------------------------------------------------------------#
ifeq ($(WITH_BORDER_ROUTER),1)
USB=1
CFLAGS += -DOSF_CONF_BR_NODE=255 # All nodes are routers
# Enable OSF Border Router
$(warning "ROUTER : OSF BORDER ROUTER")
MODULES += $(CONTIKI_NG_SERVICES_DIR)/osf-border-router
# Enable access to global internet via Internet Service Node
ifneq ($(ISN),)
	CFLAGS += -DOSF_CONF_BR_ISN=$(ISN)
    $(warning "ISN=$(ISN)")
endif
endif

#----------------------------------------------------------------------------#
# Serial over USB
##---------------------------------------------------------------------------#
ifneq ($(USB),)
NRF52840_NATIVE_USB=1
$(warning "NRF52840_NATIVE_USB=1")
CFLAGS += -DNRF52840_NATIVE_USB=1 -DUSB_SERIAL_CONF_ENABLE
MODULES += $(CONTIKI_NG_SERVICES_DIR)/slip-cmd
# Extra modules for testing
ifeq ($(WITH_SHELL), 1)
    $(warning "WITH_SHELL=1")
    MODULES += $(CONTIKI_NG_SERVICES_DIR)/shell
endif
endif

#----------------------------------------------------------------------------#
# nRF21540 FEM
#----------------------------------------------------------------------------#
ifneq ($(FEM),)
    CFLAGS += -DUSE_FEM=$(FEM)
endif
ifeq ($(FEM), 1)    
    $(warning "FEM=1")
    NETSTACK_PA = MAKE_PA_NRF21540PA
    ifneq ($(FEM_MODE),)
    CFLAGS += -DFEM_MODE=$(FEM_MODE) # PA_TX_Plus20dBm, PA_TX_Plus10dBm
    $(warning "FEM_MODE=$(FEM_MODE)")
    endif
    ifneq ($(FEM_ANT_RX),)
    CFLAGS += -DFEM_ANT_RX=$(FEM_ANT_RX) # PA_LNA_ANT1, PA_LNA_ANT2
    $(warning "FEM_ANT_RX=$(FEM_ANT_RX)")
    endif
    ifneq ($(FEM_ANT_TX),)
    CFLAGS += -DFEM_ANT_TX=$(FEM_ANT_TX) # PA_LNA_ANT1, PA_LNA_ANT2
    $(warning "FEM_ANT_TX=$(FEM_ANT_TX)")
    endif
endif

#----------------------------------------------------------------------------#
# HW variant.
#----------------------------------------------------------------------------#
ifeq ($(HWVAR), 1)    
    $(warning "HWVAR=1")
    CFLAGS += -DHWVAR=1
endif

include $(CONTIKI)/Makefile.include

