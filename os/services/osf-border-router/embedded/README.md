This the OSF border router that runs embedded in a node. The node runs a full
6LoWPAN stack, but without any routing layer (as OSF floods the network). It
interfaces to the outside world via a serial line. On the host Operating System,
`tunslip6` is used to create a tun interface and bridge it to the OSF border
router. This is achieved with makefile targets `connect-router`.
