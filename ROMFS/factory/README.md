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
