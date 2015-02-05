obj-m := ttt-module.o
ttt-module-y := ttt-game ttt-opponent

build:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
