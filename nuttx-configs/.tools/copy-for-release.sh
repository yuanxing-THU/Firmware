#!/usr/bin/env bash

CUSTOM_NUTTX_CONFIGS=$1
BOARD=$2
DEST_NUTTX_CONFIGS=$3


SOURCE=$CUSTOM_NUTTX_CONFIGS/${BOARD%-release}
TARGET=$DEST_NUTTX_CONFIGS/$BOARD

if ! [ -d "$SOURCE" -a -d "$DEST_NUTTX_CONFIGS" ]
then
	echo Usage: $0 .../nuttx-configs board .../NuttX/nuttx/configs
	exit 1
fi

mkdir -p $TARGET \
&& cp -af $SOURCE/. $TARGET/. \
&& echo "CONFIG_NSH_AIRDOG_DISABLE_CONSOLE_SESSION=y" >> $TARGET/nsh/defconfig
