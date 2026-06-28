obj-m += pet.o

pet-objs := \
    pet_main.o \
    pet_device.o \
    pet_proc.o \
    pet_state.o \
    pet_log.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
