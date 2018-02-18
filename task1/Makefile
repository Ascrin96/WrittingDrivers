obj-m += task1.o
all: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

task1:
	sudo dmesg -C
	sudo insmod task1.ko
	sudo rmmod task1.ko
	dmesg