# Factory checks

#### eMMC

```sh
nsh> sh /etc/extras/emmc
---
umount /fs/microsd
mkfatfs /dev/mmcsd0
mount -t vfat /dev/mmcsd0 /fs/microsd
===
OK
nsh>
```


#### FRAM

```sh
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


#### IO chip

```sh
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


#### BL600

```sh
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


#### BT740

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


#### GPS

Here are two commands required:
* `sh /etc/extras/settings`
* `sh /etc/extras/gps`

AIRFMU output (fast case):
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

ALMAIN output (fast case):
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

ALMAIN output (slow case):
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
