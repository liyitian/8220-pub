obj-m += mymod.o

default:
	$(MAKE) -C /usr/src/linux M=$(PWD) modules
clean: 
	rm *.ko
	rm *.o
	rm *.mod.c
	rm *.symvers
	rm *.order
