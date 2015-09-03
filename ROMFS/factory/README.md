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
