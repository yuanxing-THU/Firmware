#!/usr/bin/env bash

CUSTOM_NUTTX_CONFIGS=$1
BOARD=$2
DEST_NUTTX_CONFIGS=$3


SOURCE=$CUSTOM_NUTTX_CONFIGS/${BOARD#Factory-}
TARGET=$DEST_NUTTX_CONFIGS/$BOARD

if ! [ -d "$SOURCE" -a -d "$DEST_NUTTX_CONFIGS" ]
then
	echo Usage: $0 .../nuttx-configs board .../NuttX/nuttx/configs
	exit 1
fi

mkdir -p $TARGET \
&& cp -af $SOURCE/. $TARGET/. \
&& sed -i \
	-e '/CONFIG_NSH_LINELEN=/s:=.*:=512:' \
	$TARGET/nsh/defconfig
