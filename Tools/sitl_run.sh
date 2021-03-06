#!/bin/bash

rc_script=$1
debugger=$2
program=$3
build_path=$4

echo SITL ARGS
echo rc_script: $rc_script
echo debugger: $debugger
echo program: $program
echo build_path: $buid_path

if [ "$#" != 4 ]
then
	echo usage: sitl_run.sh rc_script debugger program build_path
	exit 1
fi

cp Tools/posix_lldbinit $build_path/src/firmware/posix/.lldbinit
cp Tools/posix.gdbinit $build_path/src/firmware/posix/.gdbinit

SIM_PID=0

if [ "$program" == "jmavsim" ]
then
	cd Tools/jMAVSim
	ant
	java -Djava.ext.dirs= -cp lib/*:out/production/jmavsim.jar me.drton.jmavsim.Simulator -udp 127.0.0.1:14560 &
	SIM_PID=echo $!
	cd ../..
elif [ "$3" == "gazebo" ]
then
	if [ -x "$(command -v gazebo)" ]
	then
		gazebo ${SITL_GAZEBO_PATH}/worlds/iris.world &
		SIM_PID=echo $!
	else
		echo "You need to have gazebo simulator installed!"
		exit 1
	fi
fi
cd $build_path/src/firmware/posix
mkdir -p rootfs/fs/microsd
mkdir -p rootfs/eeprom
touch rootfs/eeprom/parameters
# Start Java simulator
if [ "$debugger" == "lldb" ]
then
	lldb -- mainapp ../../../../$rc_script
elif [ "$debugger" == "gdb" ]
then
	gdb --args mainapp ../../../../$rc_script
else
	./mainapp ../../../../$rc_script
fi

if [ "$3" == "jmavsim" ]
then
	kill -9 $SIM_PID
elif [ "$3" == "gazebo" ]
then
	kill -9 $SIM_PID
fi
