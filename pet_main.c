#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include "pet.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tony");
MODULE_DESCRIPTION("Tamagotchi Linux character device");
DEFINE_MUTEX(pet_lock);


struct Pet pet;


static int __init pet_init(void)
{
    int ret;

    // initialize pet state
    pet_state_init();

    ret = pet_device_register();
    if (ret)
        return ret;

    ret = pet_proc_register();
    if (ret) {
        pet_device_unregister();
        return ret;
    }

    pet_timer_start();

    return 0;
}


static void __exit pet_exit(void) {
    pet_timer_stop();
    pet_device_unregister();
    pet_proc_unregister();
    printk(KERN_INFO "pet: /dev/pet removed\n");
    
}

module_init(pet_init);
module_exit(pet_exit);
