# Open-SF (OSF): An Open-Source Project for Multi-PHY Synchronous Flooding Protocols

## Intro

Open-SF (OSF) is an open-source stack for the development of multi-PHY Synchronous Flooding (SF) protocols, presented at EWSN '22. Based on the [Contiki-NG](https://github.com/contiki-ng/contiki-ng) low-power operating system, OSF was originally forked from [BlueFlood](https://github.com/iot-chalmers/BlueFlood). Currently OSF only supports the [Nordic nRF52840](https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk) (Contiki-NG has support for both the nRF52840-DK and the nRF52840-dongle).

OSF *completely* replaces BlueFlood's polling-based radio driver with an interrupt-based driver, allowing fine-grained timer control of the state machine. Furthermore OSF supports a number of easy-to-use tools for benchmarking and testing with the [D-Cube](https://iti-testbed.tugraz.at/wiki/index.php/Main_Page) low-power wireless testbed.

## Features

- Full customization of flooding rounds e.g., NTX, PWR, NSLOTS etc. (including runtime multi-PHY configuration).
- Extension framework for developing SF radio-driver enhancements.
- Extension framework for modifying existing SF protocols.
- Protocol framework for developing new SF protocols.
- One-to-Many dissemination communication.
- Many-to-One collection communication.
- Slot-based channel hopping.
- Robust Flooding (RoF) SF primitive (i.e. Tx-Tx-Tx).
- Glossy SF primitive (i.e. Rx-Tx-Rx).
- Nordic nRF42850 helper for tilix support (Linux only).
- Full support and integration with the [D-Cube](https://iti-testbed.tugraz.at/wiki/index.php/Main_Page) testbed API using [Contiki-NG-DCube](https://github.com/mbaddeley/contiki-ng-dcube).

## EWSN 2022 Paper

Full details of the OSF architecture, along with extensive benchmarking results, can be found in the [EWSN 2022 paper](https://michaelbaddeley.files.wordpress.com/2022/07/baddeley2022osf.pdf).

## How to Cite OSF

```
@inproceedings{baddeley2022osf,
  author= {Baddeley, Michael and Gyl, Yevgen and Schuss, Markus and Ma, Xiaoyuan and Boano, Carlo Alberto},
  title= {{OSF: An Open-Source Framework for Synchronous Flooding over Multiple Physical Layers}},
  booktitle= {Proceedings of the $19^{th}$ International Conference on Embedded Wireless Systems and Networks ({EWSN})},
  month= oct,
  year=2022,
};
```

## Relevant Folders

An example application:
- examples/osf

Main OSF file structure:
- os/net/mac/osf

For testing purposes:
- os/services/deployment
- os/services/testbed
- tools/dcube
- tools/nrf

## Outstanding TODOs

While OSF supports a fair number of features, there are still many things on the TODO list! Feel free to pick up any of the issues, and if you're building on OSF for your own projects then please consider creating a Pull Request :)

---

## How to Use

OSF has been tested and developed on Ubuntu. We strongly recommend using a native Ubuntu installation, and not a VM. Details on how to install the arm compiler and other tools needed for the nRF boards can be found on the [Contiki-NG Wiki](https://github.com/contiki-ng/contiki-ng/wiki/Platform-nrf52840).

### nrf-helper.sh

You can configure this using the *nrf-helper* tool to automatically generate a deployment mapping of all boards connected to your machine.
```
$ cd tools/nrf
$ ./nrf-helper.sh -info -s -d
```

You can find your board IDs in the deployment mapping generated `os/services/deployment/nulltb/deployment-map-nulltb.c`, or by using following command.
```
$ ./nrf-helper -info
```

You can also use the *nrf-helper* tool to automatically spawn serial terminals if using [tilix](https://gnunn1.github.io/tilix-web/).
```
$ sudo apt install tilix
$ tilix
$ cd <path-to-osf>/tools/nrf
$ ./nrf-helper -t
```

### Hello World
OSF contains a simple hello world application that sends and receives messages at the application layer. You can specify the source node with SRC=\* and broadcast to all other nodes with DST=255.

```
$ cd examples/osf
$ make clean TARGET=nrf52840 && make -j16 node.upload-all TARGET=nrf52840 BOARD=dk DEPLOYMENT=nulltb SRC=1 DST=2 DEPLOYMENT=nulltb HELLO_WORLD=1 PERIOD=1000 CHN=1 LOGGING=1 GPIO=1 LEDS=1 NTX=6 NSLOTS=6 PWR=ZerodBm PROTO=OSF_PROTO_BCAST DCUBE=1 PHY=PHY_BLE_500K

```

### Testbed Service

For testing purposes, you can use the NULLTB dummy testbed setup in *services/testbed/..* to automatically generate, send, and receive random packet data between boards. This testbed service can also be used to interface with the D-Cube testbed. For more information on this, please check out [Contiki-NG-DCube](https://github.com/mbaddeley/contiki-ng-dcube).

The following command will allow you to compile OSF, flash it to any number of nRF52840-dk boards you have connected to your machine, and observe dummy packets sent from one board (SRC=\*) to all other boards (DST=255).

```
$ cd examples/osf
$ make clean TARGET=nrf52840 && make -j16 node.upload-all TARGET=nrf52840 BOARD=dk SRC=1 DST=255 TESTBED=nulltb DEPLOYMENT=nulltb LENGTH=8 PERIOD=200 CHN=1 LOGGING=1 GPIO=1 LEDS=1 PROTO=OSF_PROTO_BCAST PHY=PHY_BLE_500K
```

### Extension Framework

OSF implements an extension framework to allow straightforward extension of the base driver and/or protocols. A couple of extensions, *random backoff* and *random NTX*, were implemented as part of EWSN '22. Both these extensions significantly improve the performance of the BLE 2M physical layer in dense scenarios, where the uncoded PHYs rarely benefit from capture effects and are significantly impacted by desynchronization.

#### Random BACKOFF

```
make clean TARGET=nrf52840 && make -j16 node.upload-all TARGET=nrf52840 BOARD=dk DEPLOYMENT=nulltb DST=1 SRC=2,3,4 TESTBED=nulltb DEPLOYMENT=nulltb LENGTH=64 PERIOD=1000 CHN=1 LOGGING=1 GPIO=1 LEDS=1 NTX=6 NSLOTS=6 NTA=4 PWR=ZerodBm PROTO=OSF_PROTO_STA DCUBE=1 PHY=PHY_BLE_2M BACKOFF=1
```

#### Random NTX

```
make clean TARGET=nrf52840 && make -j16 node.upload-all TARGET=nrf52840 BOARD=dk DEPLOYMENT=nulltb DST=1 SRC=2,3,4 TESTBED=nulltb DEPLOYMENT=nulltb LENGTH=64 PERIOD=1000 CHN=1 LOGGING=1 GPIO=1 LEDS=1 NTX=6 NSLOTS=6 NTA=4 PWR=ZerodBm PROTO=OSF_PROTO_STA DCUBE=1 PHY=PHY_BLE_2M RNTX=1
```

### Multi-PHY

OSF supports the development of multi-PHY protocols, allowing exploitation of multiple PHYS *in the same protocol*, as well as dynamic selection and (re)configuration of the PHY *at runtime*. OSF provides a naieve implmentation of such a protocol, the details of which can be found in the [paper](https://michaelbaddeley.files.wordpress.com/2022/07/baddeley2022osf.pdf).

```
make clean TARGET=nrf52840 && make -j16 node.upload-all TARGET=nrf52840 BOARD=dk DEPLOYMENT=nulltb DST=1 SRC=2,3,4 TESTBED=nulltb DEPLOYMENT=nulltb LENGTH=64 PERIOD=1000 CHN=1 LOGGING=1 GPIO=1 LEDS=1 NTX=6 NSLOTS=6 NTA=4 PWR=ZerodBm PROTO=OSF_PROTO_STA DCUBE=1 PHY=PHY_BLE_2M MPHY=1
```

---   

## Makeargs

OSF has the following make options. Default options are in **bold**.

#### Deployment and Data Generation
| ARG                     | Description |
|-------------------------| ----------- |
| HELLO_WORLD=1/0 | Hello World application |
| DEPLOYEMENT=nulltb/dube | D-Cube testbed integration OR dummy testbed integration |
| TESTBED=nulltb/dcube | D-Cube testbed deployment IDs OR dummy testbed deployment IDs (using *nrf-helper.sh*) |
| SRC=1,2, ... * | ID's (as per deployment mapping) of source nodes |
| DST=1,2, ... * | ID's (as per deployment mapping) of destination nodes |
| FWD=1,2, ... * | ID's (as per deployment mapping) of forwarding nodes |

#### Data
| ARG                     | Description |
|-------------------------| ----------- |
| LENGTH=* (**8**) | (Max) length of data in Bytes) |

#### Basic OSF Configuration
| ARG                     | Description |
|-------------------------| ----------- |
| TS=* | ID of timesync node. Will default to 1st SRC w/ BCAST or 1st DST w/ STA |
| PROTO=**OSF_PROTO_BCAST**/OSF_PROTO_STA  | Protocol choice. Broadcast (i.e., just one S round with a payload) or STA (i.e., Crystal) |
| PERIOD=* (**500**) | Epoch periodicity in ms |
| NTX=* (**6**) | Max # of Tx in a round |
| NSLOTS=* (**12**) | Max # of slots in a round |
| PHY=PHY_BLE_2M/1M/**500K**/125K / PHY_IEEE | Underlying physical layer |
| PRIMITIVE=**OSF_PRIMITIVE_ROF**/OSF_PRIMITIVE_GLOSSY | Underlying SF primitive (i.e., RxTxTx or RxTxRxTx) |
| CHN=**1**/0 | Per-slot channel hopping (0 = single channel, 1 = seeded, 2 = only use sync channels) |
| PWR=Neg20dBm -> Pos8dBm (**ZerodBm**) | Transmission power (N.B. nRF Neg40dBm doesn't work!) |

#### Protocol Configuration
| ARG                     | Description |
|-------------------------| ----------- |
| NTA=* (**4**) | Number of TA pairs in STA protocol (Crystal) |
| EMPTY=* (**0**) | Signal an early exit from STA protocol after *N* empty rounds (to make STA more like Crystal) |
| TOG=**0**/1 | Bit toggling to indicate reception on the ACK round in STA |
| ALWAYS_ACK=**0**/1 | **Always** ACK  (i.e., every epoch). To be used in conjunction with TOG=0/1 |
| MPHY=**0**/1 | Naieve [EWSN '22 multi-PHY protocol](https://michaelbaddeley.files.wordpress.com/2022/07/baddeley2022osf.pdf) (in conjunction with STA protocol) |

#### Protocol Extensions
| ARG                     | Description |
|-------------------------| ----------- |
| BACKOFF=**0**/1 | Random backoff for STA protocol |
| ND=**0**/1 | Noise detection extension |

#### Driver Extensions
| ARG                     | Description |
|-------------------------| ----------- |
| RNTX=**0**/1 | Random NTX for *any* protocol |

#### Testing and Debug
| ARG                     | Description |
|-------------------------| ----------- |
| LOGGING=**1**/0 | OSF logging. (see osf-logging.h) |
| GPIO=**1**/0 | OSF GPIOs (see osf-debug.h) |
| LEDS=**1**/0 | OSF LEDs |
| MISS_RXS=**0**/1 | Miss *N* receptions in a round |
| TEST_NODE=**0**/1 | Node ID under test for MISS_RXS |
