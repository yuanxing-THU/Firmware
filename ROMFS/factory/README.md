# Factory checks

## General convention

All the scripts put `---` at the start and `===` at the end,
following `OK` or `FAIL` for binary checks.
When you need to analyze the output yourself,
the output is either `ok, check the values` or `FAIL`.

Commands are run via _nsh_ shell. Its command line prompt is `nsh> \033[K`.
Therefore each command output is followed by the sequence.


## AIRFMU checks

* [BL600](#bl600)
* [eMMC](#emmc)
* [FRAM](#fram)
* [IO chip](#io-chip)
* [Sensor Barometer MS5611](#sensor-barometer-ms5611)


## AIRFMU + AIRSEN + AIRBOT + AIRMAG + AIRBGC + Battery

* [BGC](#bgc)
* [BT740](#bt740)
* [GPS](#gps)
* [Sensors](#sensors)


## ALMAIN checks

* [ADC](#adc)
* [BT740](#bt740)
* [eMMC](#emmc)
* [FRAM](#fram)
* [GPS](#gps)
* [Sensors](#sensors)


## Alfabetical list

* [ADC](#adc)
* [BGC](#bgc)
* [BL600](#bl600)
* [BT740](#bt740)
* [eMMC](#emmc)
* [FRAM](#fram)
* [GPS](#gps)
* [IO chip](#io-chip)
* [Sensors](#sensors)
  * [Barometer MS5611](#sensor-barometer-ms5611)


### ADC

Command is `sh /etc/extras/adc`.


#### AirDog

_TODO_.


#### AirLeash

Channel 2 gives you current board voltage as seen by ADC.

```
nsh> sh /etc/extras/adc
---
adc
<adc> init done
sensors_switch factory-adc-check
channel 2 raw 0xaf8 value 4.11
===
ok, check the values.
nsh>
```


### BGC

Successfull case:

```
nsh> sh /etc/extras/bgc
---
bgc test
[BGC_uart] Discover_attributes
[BGC_uart] trying attributes: speed=256000 parity=0
[BGC_uart] Get_board_info
[BGC_uart] Get_board_info - SBGC_CMD_BOARD_INFO
[BGC_uart] board ver        = 3.1
[BGC_uart] firmware ver     = 2.43b9
[BGC_uart] debug mode       = 0
[BGC_uart] board features   = 0000
[BGC_uart] connection flags = 00
[BGC_uart] discovered attributes: speed=256000 parity=0
[BGC] discovered BGC_uart attributes: speed=256000 parity=0
[BGC] sent CMD_TRIGGER_PIN 18/1
[BGC Factory Check] version ok.
===
OK
nsh>
```

No communication:

```
nsh> sh /etc/extras/bgc
---
bgc test
[BGC_uart] Get_board_info
[BGC_uart] Get_board_info
[BGC_uart] Get_board_info
[BGC] couldn't use old BGC_uart attributes: speed=256000 parity=0
[BGC_uart] Discover_attributes
[BGC_uart] trying attributes: speed=256000 parity=0
[BGC_uart] Get_board_info
[BGC_uart] Get_board_info
[BGC_uart] Get_board_info
[BGC_uart] trying attributes: speed=115200 parity=0
[BGC_uart] Get_board_info
...
[BGC_uart] Get_board_info
[BGC_uart] trying attributes: speed=57600 parity=16
[BGC_uart] Get_board_info
[BGC_uart] Get_board_info
[BGC_uart] Get_board_info
[BGC] failed to discover BGC_uart attributes
[BGC Factory Check] communication failed.
===
FAIL
nsh>
```


### BL600

Command is `sh /etc/extras/bl600`.

```
nsh> sh /etc/extras/bl600
---
mobile mode at
mobile firmware-version /dev/ttyS2
0# AT I 3
: 10    3       1.8.88.0
: 00
required version: 1 8 88 0
modile's version: 1 8 88 0
ready to work.
===
OK
nsh>
```


### BT740

Here are two commands required:
* `sh /etc/extras/settings`
* `sh /etc/extras/bt740`

AIRFMU output:
```
nsh> sh /etc/extras/settings
nsh> sh /etc/extras/bt740
---
bluetooth21 firmware-version /dev/ttyS1
bt21_io starting ...
bt21_io started.
bluetooth21:   18811014: BT required version: x.x.x.277
bluetooth21:   18841232: BT firmware version: 2.3.1.277
bt21_io stop requested.
bt21_io stopped.
===
OK
nsh>
```

ALMAIN output:
```
nsh> sh /etc/extras/settings
nsh> sh /etc/extras/bt740
---
bluetooth21 firmware-version /dev/ttyS0
bt21_io starting ...
bt21_io started.
bluetooth21:  221039267: BT required version: x.x.x.277
bluetooth21:  221069237: BT firmware version: 2.3.1.277
bt21_io stop requested.
bt21_io stopped.
===
OK
nsh>
```


### eMMC

Command is `sh /etc/extras/emmc`.

Very first looks like

```
nsh> sh /etc/extras/emmc
---
test -e /fs/microsd
mkfatfs /dev/mmcsd0
mount -t vfat /dev/mmcsd0 /fs/microsd
===
OK
nsh>
```

Being run for second time

```
nsh> sh /etc/extras/emmc
---
test -e /fs/microsd
umount /fs/microsd
mkfatfs /dev/mmcsd0
mount -t vfat /dev/mmcsd0 /fs/microsd
===
OK
nsh>
```


### FRAM

Command is `sh /etc/extras/fram`.

```
nsh> sh /etc/extras/fram
---
mtd start
mtd erase
Erasing /fs/mtd_params
Erased 8192 bytes
Erasing /fs/mtd_waypoints
Erased 8192 bytes
mtd status
mtd: Flash Geometry:
  blocksize:      512
  erasesize:      512
  neraseblocks:   32
  No. partitions: 2
  Partition size: 16 Blocks (8192 bytes)
  TOTAL SIZE: 16 KiB
===
OK
nsh>
```


### GPS

Here are two commands required:
* `sh /etc/extras/settings`
* `sh /etc/extras/gps`


#### AIRFMU output, quick case

```
nsh> sh /etc/extras/settings
nsh> sh /etc/extras/gps
---
Warning: It could take long time!

gps start -d /dev/ttyS3
[gps] starting
gps: using NAV-PVT
gps: GPS enabled 1 8 24
gps: SBAS enabled 1
gps: BDS enabled 0
gps: QZSS enabled 1
gps: GLONASS enabled 0
gps: Gps configuration ok
gps: VER hash 0x38eee1b4
gps: VER hw  "  00080000"
gps: VER sw  "                  2.01 (75331)"
gps: VER ext "                  PROTVER 15.00"
gps: VER ext "          GPS;SBAS;GLO;BDS;QZSS"
gps: module found: UBX

gps test
gps: PASS

gps stop
gps: module lost
gps: exiting
===
OK
nsh>
```


#### ALMAIN output, quick case

```
nsh> sh /etc/extras/settings
nsh> sh /etc/extras/gps
---
Warning: It could take long time!

gps start -d /dev/ttyS1
[gps] starting
gps: using NAV-PVT
gps: GPS enabled 1 8 24
gps: SBAS enabled 1
gps: BDS enabled 0
gps: QZSS enabled 1
gps: GLONASS enabled 0
gps: Gps configuration ok
gps: VER hash 0x38eee1b4
gps: VER hw  "  00080000"
gps: VER sw  "                  2.01 (75331)"
gps: VER ext "                  PROTVER 15.00"
gps: VER ext "          GPS;SBAS;GLO;BDS;QZSS"
gps: module found: UBX

gps test
gps: PASS

gps stop
gps: module lost
gps: exiting
===
OK
nsh>
```

#### Slow case

```
nsh> sh /etc/extras/settings
nsh> sh /etc/extras/gps
---
Warning: It could take long time!

gps start -d /dev/ttyS1
[gps] starting
gps: ubx checksum err
gps: ubx checksum err
gps: ubx checksum err

gps test
gps: No communication.
gps: ubx checksum err
gps: ubx checksum err
gps: ubx checksum err

gps test
gps: No communication.
gps: ubx msg 0x0107 invalid len 46556
gps: ubx msg 0x0107 invalid len 46428
gps: ubx msg 0x0107 invalid len 22364
gps: ubx msg 0xe16d len 20652 unexpected
gps: ubx disabling msg 0xe16d

gps test
gps: No communication.
gps: ubx msg 0x0107 invalid len 46428
gps: ubx checksum err
gps: ubx msg 0x0107 invalid len 46556
gps: ubx checksum err
gps: ubx checksum err
gps: ubx checksum err

gps test
gps: No communication.
gps: ubx checksum err
gps: ubx msg 0x0107 invalid len 4950
gps: ubx checksum err
gps: ubx msg 0x0107 invalid len 22108
gps: ubx checksum err
gps: ubx msg 0x0107 invalid len 22108

gps test
gps: No communication.
gps: ubx checksum err
gps: ubx msg 0x0107 invalid len 57436
gps: ubx msg 0x0107 invalid len 4951
gps: ubx checksum err
gps: using NAV-PVT
gps: GPS enabled 1 8 24
gps: SBAS enabled 1
gps: BDS enabled 0
gps: QZSS enabled 1
gps: GLONASS enabled 0
gps: Gps configuration ok
gps: VER hash 0x38eee1b4
gps: VER hw  "  00080000"
gps: VER sw  "                  2.01 (75331)"
gps: VER ext "                  PROTVER 15.00"
gps: VER ext "          GPS;SBAS;GLO;BDS;QZSS"
gps: module found: UBX

gps test
gps: PASS

gps stop
gps: module lost
gps: exiting
===
OK
nsh>
```

### IO chip

```
nsh> sh /etc/extras/px4io
---
test -f /etc/extras/px4io-v2_default.bin
px4io forceupdate 14662 /etc/extras/px4io-v2_default.bin
px4io: px4io is not started, still attempting upgrade
[PX4IO] using firmware from /etc/extras/px4io-v2_default.bin
[PX4IO] bad sync 0xff,0xff
[PX4IO] found bootloader revision: 4
[PX4IO] erase...
[PX4IO] programming 57372 bytes...
[PX4IO] verify...
[PX4IO] update complete
px4io checkcrc /etc/extras/px4io-v2_default.bin
CRCs match
===
OK
nsh>
```


### Sensors

Here are two commands required:
* `unset SENS`
* `sh /etc/extras/sens`

AirDog output:

```
nsh> unset SENS
nsh> sh /etc/extras/sens
---
sensors_probe
dev #1 send 2:  8f 00
dev #1 recv 2:  ff d4
l3gd20: ok
dev #2 send 2:  8f 00
dev #2 recv 2:  ff 49
lsm303d: ok
dev #4 send 2:  8c 00
dev #4 recv 2:  00 58
mpu6000: ok
dev #3 send 1:  1e
dev #3 recv 1:  fe
dev #3 send 3:  a0 00 00
dev #3 recv 3:  00 00 00
dev #3 send 3:  a2 00 00
dev #3 recv 3:  00 00 00
dev #3 send 3:  a4 00 00
dev #3 recv 3:  00 00 00
dev #3 send 3:  a6 00 00
dev #3 recv 3:  fe 83 5f
ms5611: ok
dev #5 send 4:  ca 00 00 00
dev #5 recv 4:  ff 48 34 33
hmc5883: ok
===
OK
nsh>
```

ALMAIN output:

```
nsh> unset SENS
nsh> sh /etc/extras/sens
---
sensors_probe
dev #1 send 2:  8f 00
dev #1 recv 2:  00 d4
l3gd20: ok
dev #2 send 2:  8f 00
dev #2 recv 2:  00 49
lsm303d: ok
dev #4 send 2:  8c 00
dev #4 recv 2:  00 58
mpu6000: ok
dev #3 send 1:  1e
dev #3 recv 1:  fe
dev #3 send 3:  a0 00 00
dev #3 recv 3:  00 00 00
dev #3 send 3:  a2 00 00
dev #3 recv 3:  00 00 00
dev #3 send 3:  a4 00 00
dev #3 recv 3:  00 00 00
dev #3 send 3:  a6 00 00
dev #3 recv 3:  fe 7e 6f
ms5611: ok
===
OK
```


#### Sensor Barometer MS5611

Here are two commands required:
* `set SENS ms5611`
* `sh /etc/extras/sens`

AirDog output:

```
nsh> set SENS ms5611
nsh> sh /etc/extras/sens
---
sensors_probe ms5611
dev #3 send 1:  1e
dev #3 recv 1:  fe
dev #3 send 3:  a0 00 00
dev #3 recv 3:  00 00 00
dev #3 send 3:  a2 00 00
dev #3 recv 3:  00 00 00
dev #3 send 3:  a4 00 00
dev #3 recv 3:  00 00 00
dev #3 send 3:  a6 00 00
dev #3 recv 3:  fe 83 5f
ms5611: ok
===
OK
nsh>
```
