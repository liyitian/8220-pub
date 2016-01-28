obj-m += mymod.o


mod:
	$(MAKE) -C /usr/src/linux M=$(PWD) modules

user:
	gcc user.c -o user

clean:
	rm -f *.ko *.o *.mod.c *.symvers *.order user
