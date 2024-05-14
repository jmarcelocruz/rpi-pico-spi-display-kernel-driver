ARCH = arm64
CROSS_COMPILE = aarch64-none-linux-gnu-
INSTALL_MOD_PATH =

module:
	${MAKE} -C external/linux  ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} M=${PWD}

modules_install:
	${MAKE} -C external/linux  ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} INSTALL_MOD_PATH=${INSTALL_MOD_PATH} M=${PWD} modules_install

clean:
	${MAKE} -C external/linux  ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} M=${PWD} clean
	rm -f rpi-4-b-pico-display.dtbo

help:
	${MAKE} -C external/linux  ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} M=${PWD} help

rpi-4-b-pico-display.dtbo: dts/rpi-4-b-pico-display.dts
	dtc -@ -I dts -O dtb $^ -o $@
