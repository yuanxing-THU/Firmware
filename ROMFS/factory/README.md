# Factory checks

## USB checks

### FMU only checks

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
