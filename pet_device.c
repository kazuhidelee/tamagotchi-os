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

static int major_number;

static struct class* pet_class = NULL;
static struct device* pet_device = NULL;



static ssize_t pet_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    char msg[128];
    int msg_len;
    mutex_lock(&pet_lock);
    msg_len = snprintf(msg, sizeof(msg),
        "Name: Mochi\nHealth: %d\nHunger: %d\nHappiness: %d\nMood: %s\n",
        pet.health, pet.hunger, pet.happiness, pet_get_mood()
    );
    mutex_unlock(&pet_lock);
    if (*offset >= msg_len) {
        return 0;
    }

    if (copy_to_user(buffer, msg, msg_len)) {
        return -EFAULT;
    }

    *offset += msg_len;
    return msg_len;
}

static ssize_t pet_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    char command[32];

    if (len > 31) {
        len = 31;
    }

    if (copy_from_user(command, buffer, len)) {
        return -EFAULT;
    }
    mutex_lock(&pet_lock);
    command[len] = '\0';

    if (strncmp(command, "feed", 4) == 0) {
        pet.hunger -= 10;
        pet.happiness += 2;
    } else if (strncmp(command, "play", 4) == 0) {
        pet.happiness += 10;
        pet.hunger += 5;
    } else if (strncmp(command, "sleep", 5) == 0) {
        pet.health += 5;
        pet.hunger += 3;
    } else if (strncmp(command, "medicine", 8) == 0){
        pet.health += 10;
    }
    pet_clamp_state();
    mutex_unlock(&pet_lock);
    return len;
}



static struct file_operations fops = {
    .read = pet_read,
    .write = pet_write,
};

int pet_device_register(void){
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "pet: failed to register major number\n");
        return major_number;
    }
    pet_class = class_create(CLASS_NAME);
    if (IS_ERR(pet_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "pet: failed to register device class\n");
        return PTR_ERR(pet_class);
    }

    pet_device = device_create(pet_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(pet_device)) {
        class_destroy(pet_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "pet: failed to create device\n");
        return PTR_ERR(pet_device);
    }
    
    return 0;
}

void pet_device_unregister(void)
{
    device_destroy(pet_class, MKDEV(major_number, 0));
    class_destroy(pet_class);
    unregister_chrdev(major_number, DEVICE_NAME);
}

