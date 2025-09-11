obj-m := driver.o

all: x86_build	# default: x86_build

clean: x86_clean RPI_clean

# x86
x86_build: driver.c
	make -C /lib/modules/6.8.0-45-generic/build M=$(PWD) modules

x86_clean:
	make -C /lib/modules/6.8.0-45-generic/build M=$(PWD) clean

# RPI
# - Ensure your Raspberry Pi kernel source is located in $(PWD)/linux
# - If it's located elsewhere, modify the -C path accordingly
RPI_build: driver.c
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(PWD)/linux M=$(PWD) modules	

RPI_clean:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(PWD)/linux M=$(PWD) clean

.PHONY: all clean x86_build x86_clean RPI_build RPI_clean
