#!/bin/bash 
# This bash script is used to compile automatically the Prusa firmware with a dedecated build enviroment and settings
# 
# Supported OS: Windows 10, Linux64 bit
#
# Linux:
#
# Windows:
# To excecute this sciprt you gonna need few things on your Windows machine
# 1. Download and install the correct (64bit or 32bit) Git version https://git-scm.com/download/win
# 2. Also follow these instructions https://gist.github.com/evanwill/0207876c3243bbb6863e65ec5dc3f058
# 3. 
# Usefull things to edit and compare your custom Firmware
# 1. Download and install current and correct (64bit or 32bit) Notepad++ version https://notepad-plus-plus.org/download
# 2. Another great tool to compare your custom mod and stock firmware is WinMerge http://winmerge.org/downloads/?lang=en
# 
# Example for MK3: open git bash and chage to your Firmware directory 
# <username>@<machinename> MINGW64 /<drive>/path
# bash win_build.sh 1_75mm_MK3-EINSy10a-E3Dv6full
#
# Example for MK25: open git bash and chage to your directory 
# gussner@WIN01 MINGW64 /d/Data/Prusa-Firmware/MK3
# bash win_build.sh 1_75mm_MK25-RAMBo13a-E3Dv6full
#
# The compiled hex files can be found in the folder above like from the example
# gussner@WIN01 MINGW64 /d/Data/Prusa-Firmware
# FW351-Build1778-1_75mm_MK25-RAMBo13a-E3Dv6full.hex
#
# Version: 1.0.1-Build_3
# Change log:
# 12 Jan 2019, 3d-gussner, Fixed "compiler.c.elf.flags=-w -Os -Wl,-u,vfprintf -lprintf_flt -lm -Wl,--gc-sections" in 'platform.txt'
# 16 Jan 2019, 3d-gussner, Build_2, Added development check to modify 'Configuration.h' to prevent unwanted LCD messages that Firmware is uknown
# 17 Jan 2019, 3d-gussner, Build_3, Check for OS Windows or Linux and use the right build enviroment

BUILD_ENV="1.0.1"
SCRIPT_PATH="$( cd "$(dirname "$0")" ; pwd -P )"
# First argument defines which varaint of the Prusa Firmware will be compiled, like
# 
if [ -z "$1" ]
 then
	echo "You have too choose a variant or ALL to compile all existing varaints."
	exit 0
 else
	VARIANT=$1
fi
# NOT IMPLEMENTED YET. Second argument defines if it is an english only version. Format EN_ONLY
EN_ONLY=$2
# Find firmware version in Configuration.h file and use it to generate the hex filename
FW=$(grep --max-count=1 "\bFW_VERSION\b" Firmware/Configuration.h | sed -e's/  */ /g'|cut -d '"' -f2|sed 's/\.//g')
# Find build version in Configuration.h file and use it to generate the hex filename
BUILD=$(grep --max-count=1 "\bFW_COMMIT_NR\b" Firmware/Configuration.h | sed -e's/  */ /g'|cut -d ' ' -f3)
# Check if the motherboard is an EINSY and if so the only one hex file will generated
MOTHERBOARD=$(grep --max-count=1 "\bMOTHERBOARD\b" Firmware/variants/$VARIANT.h | sed -e's/  */ /g' |cut -d ' ' -f3)
# Check development status
DEV_CHECK=$(grep --max-count=1 "\bFW_VERSION\b" Firmware/Configuration.h | sed -e's/  */ /g'|cut -d '"' -f2|sed 's/\.//g'|cut -d '-' -f2)
if [ "$DEV_CHECK" == "$FW" ] ; then
	DEV_STATUS="GOLD"
else 
	if [[ "$DEV_CHECK" == "RC1"  ||  "$DEV_CHECK" == "RC2" ]] ; then
		DEV_STATUS="RC"
	else
		DEV_STATUS="UNKNOWN"
	fi
fi

# List few useful data
echo $SCRIPT_PATH
echo $VARIANT
echo "Firmware: " $FW
echo "Build #" $BUILD
echo "Dev Check:" $DEV_CHECK
echo "DEV Status:" $DEV_STATUS
echo $MOTHERBOARD
echo $OS " :  " $OSTYPE
echo $2

#### Start prepare building

#Check if build exists and creates it if not
if [ ! -d "../build-env" ]; then
    mkdir ../build-env || exit 1
fi
cd ../build-env || exit 2

# Check if PF-build-env-<version> exists and downloads + creates it if not
# The build enviroment is based on the Arduino IDE 1.8.5 portal vesion with some changes

if [ $OS == "Windows_NT" ]; then
	if [ ! -f "PF-build-env-Win-$BUILD_ENV.zip" ]; then
		echo "Downloding Windows build enviroment..."
		sleep 5
		wget https://github.com/3d-gussner/PF-build-env/releases/download/Win-$BUILD_ENV/PF-build-env-Win-$BUILD_ENV.zip || exit 3
		#cp -f ../../PF-build-env/PF-build-env-Win-$BUILD_ENV.zip PF-build-env-Win-$BUILD_ENV.zip || exit3
	fi
	if [ ! -d "../PF-build-env-$BUILD_ENV" ]; then
		echo "Unzipping Windows build enviroment..."
		sleep 5
		unzip PF-build-env-Win-$BUILD_ENV.zip -d ../PF-build-env-$BUILD_ENV || exit 4
	fi
	
fi

if [ $OS == "Linux" ]; then
	if [ ! -f "PF-build-env-Linux64-$BUILD_ENV.zip" ]; then
		echo "Downloading Linux 64 build enviroment..."
		sleep 5
		wget https://github.com/mkbel/PF-build-env/releases/download/$BUILD_ENV/PF-build-env-Linux64-$BUILD_ENV.zip || exit 3
	fi

	if [ ! -d "../../PF-build-env-$BUILD_ENV" ]; then
		echo "Unzipping Linux build enviroment..."
		sleep 5
		unzip PF-build-env-Linux64-$BUILD_ENV.zip -d ../../PF-build-env-$BUILD_ENV || exit 4
	fi
fi	


#Set BUILD_ENV_PATH
cd ../PF-build-env-$BUILD_ENV || exit 5
BUILD_ENV_PATH="$( pwd -P )"

cd ..

#Checkif BUILD_PATH exisits and if not creates it
if [ ! -d "Prusa-Firmware-build" ]; then
    mkdir Prusa-Firmware-build  || exit 6
fi

#Set the BUILD_PATH for Arduino IDE
cd Prusa-Firmware-build || exit 7
BUILD_PATH="$( pwd -P )"

#Prepare Firmware to be compiled by copying variant as Configuration_prusa.h
if [ ! -f "$SCRIPT_PATH/Firmware/Configuration_prusa.h" ]; then
    cp -f $SCRIPT_PATH/Firmware/variants/$VARIANT.h $SCRIPT_PATH/Firmware/Configuration_prusa.h || exit 8
fi
	#if [ ! -z "$EN_ONLY" ]; then

	#Set FW_DEV_VERSION to development status
	
#Check if compiler flags are set to Prusa specific needs for the rambo board.
if [ $OS == "Windows_NT" ]; then
	RAMBO_PLATFORM_FILE="rambo/hardware/avr/1.0.1/platform.txt"
	COMP_FLAGS="compiler.c.elf.flags={compiler.warning_flags} -Os -g -flto -fuse-linker-plugin -Wl,-u,vfprintf -lprintf_flt -lm -Wl,--gc-sections"
	CHECK_FLAGS=$(grep --max-count=1 "$COMP_FLAGS" $BUILD_ENV_PATH/portable/packages/$RAMBO_PLATFORM_FILE)
	if [ -z "$CHECK_FLAGS" ]; then
		echo "Compiler flags not found, adding flags"
		if [ ! -f $BUILD_ENV_PATH/portable/packages/$RAMBO_PLATFORM_FILE.bck ]; then
			echo "making a backup"
			ls $BUILD_ENV_PATH/portable/packages/rambo/hardware/avr/1.0.1/
			cp -f $BUILD_ENV_PATH/portable/packages/$RAMBO_PLATFORM_FILE $BUILD_ENV_PATH/portable/packages/$RAMBO_PLATFORM_FILE.bck
		fi
		echo $COMP_FLAGS >> $BUILD_ENV_PATH/portable/packages/$RAMBO_PLATFORM_FILE
	else
		echo "Compiler flags are set in rambo platform.txt" $CHECK_FLAGS
	fi
fi	

#### End of Prepare building
	
#### Start building
	
export ARDUINO=$BUILD_ENV_PATH
echo $BUILD_ENV_PATH
export BUILDER=$ARDUINO/arduino-builder

sleep 5
#read -p "Press any Enter..."

if [ $OS == "Windows_NT" ]; then
	echo "Start to build Prusa Firmware under Windows..."
	sleep 5
	$BUILDER -dump-prefs -logger=machine -hardware $ARDUINO/hardware -hardware $ARDUINO/portable/packages -tools $ARDUINO/tools-builder -tools $ARDUINO/hardware/tools/avr -tools $ARDUINO/portable/packages -built-in-libraries $ARDUINO/libraries -libraries $ARDUINO/portable/sketchbook/libraries -fqbn=rambo:avr:rambo -ide-version=10805 -build-path=$BUILD_PATH -warnings=all -verbose $SCRIPT_PATH/Firmware/Firmware.ino || exit 9
	$BUILDER -compile -logger=machine -hardware $ARDUINO/hardware -hardware $ARDUINO/portable/packages -tools $ARDUINO/tools-builder -tools $ARDUINO/hardware/tools/avr -tools $ARDUINO/portable/packages -built-in-libraries $ARDUINO/libraries -libraries $ARDUINO/portable/sketchbook/libraries -fqbn=rambo:avr:rambo -ide-version=10805 -build-path=$BUILD_PATH -warnings=all -verbose $SCRIPT_PATH/Firmware/Firmware.ino || exit 10
fi
if [ $OS == "Linux" ] ; then
	echo "Start to build Prusa Firmware under Linux 64..."
	sleep 5
	$BUILD_ENV_PATH/arduino $SCRIPT_PATH/Firmware/Firmware.ino --verify --board rambo:avr:rambo --pref build.path=$BUILD_PATH || exit 9
fi

# Check if it is NOT english only build
MULTI_LANGUAGE_CHECK=$(grep --max-count=1 "\/\/\#define LANG_MODE" $SCRIPT_PATH/Firmware/config.h|sed -e's/  */ /g'|cut -d ' ' -f3)

if [ "$MULTI_LANGUAGE_CHECK" ==  "0" ]; then
	echo $MULTI_LANGUAGE_CHECK
	echo "Building mutli language firmware"
	echo ""
	sleep 5
	cd $SCRIPT_PATH/lang
	./config.sh || exit 11
	# build languages
	./lang-build.sh || exit 12
	# Combine compiled firmware with languages 
	./fw-build.sh || exit 13
	# If the motherboard is an EINSY just copy one hexfile
	if [ "$MOTHERBOARD" = "BOARD_EINSY_1_0a" ]; then
		echo "Build multi language firmware for MK3/Einsy board"
		cp -f firmware.hex ../../FW$FW-Build$BUILD-$VARIANT.hex
	else
		echo "Building multi language firmware for MK2.5/miniRAMbo board"
		cp -f firmware_cz.hex ../../FW$FW-Build$BUILD-$VARIANT-cz.hex
		cp -f firmware_de.hex ../../FW$FW-Build$BUILD-$VARIANT-de.hex
		cp -f firmware_es.hex ../../FW$FW-Build$BUILD-$VARIANT-es.hex
		cp -f firmware_fr.hex ../../FW$FW-Build$BUILD-$VARIANT-fr.hex
		cp -f firmware_it.hex ../../FW$FW-Build$BUILD-$VARIANT-it.hex
		cp -f firmware_pl.hex ../../FW$FW-Build$BUILD-$VARIANT-pl.hex
	fi
	# Cleanup after build
	./fw-clean.sh || exit 14
	./lang-clean.sh || exit 15
else
	echo "English only firmware build."
	cp -f $BUILD_PATH/Firmware.ino.hex ../FW$FW-Build$BUILD-$VARIANT-EN_ONLY.hex || exit 16
fi
# Cleanup Firmware
rm $SCRIPT_PATH/Firmware/Configuration_prusa.h || exit 17

# Cleanup compiler flags are set to Prusa specific needs for the rambo board.
echo ""
echo "Restore platform.txt"
echo ""
cp -f $BUILD_ENV_PATH/portable/packages/$RAMBO_PLATFORM_FILE.bck $BUILD_ENV_PATH/portable/packages/$RAMBO_PLATFORM_FILE

# Switch to hex path and list build files
cd $SCRIPT_PATH
cd ..
echo ""
echo "List all build hex files:"
ls FW*.hex
echo ""
echo ""
echo "Build done, please use Slic3rPE > 1.41.0 to upload the firmware"
echo "more information how to flash firmware https://www.prusa3d.com/drivers/"
#### End building
