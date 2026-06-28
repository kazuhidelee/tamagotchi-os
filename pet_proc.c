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

static struct proc_dir_entry *pet_proc_entry = NULL;


static int my_proc_show(struct seq_file *m, void *v) {
    mutex_lock(&pet_lock);
    seq_printf(m,
    "Name: Mochi\nHealth: %d\nHunger: %d\nHappiness: %d\nMood: %s\n",
    pet.health, pet.hunger, pet.happiness, pet_get_mood());
    mutex_unlock(&pet_lock);
    return 0;
}

static int my_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, my_proc_show, NULL);
}


static const struct proc_ops my_proc_fops = {
    .proc_open    = my_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

int pet_proc_register(void)
{
    pet_proc_entry = proc_create(
        "tamagotchi_proc",
        0,
        NULL,
        &my_proc_fops
    );

    // TODO:
    // Check if proc_create failed.
    // Print an error with printk().
    // Return an appropriate error code.
    if (!pet_proc_entry) {
        printk(KERN_ALERT "pet: failed to create proc\n");
        return -ENOMEM;
    }

    return 0;
}

void pet_proc_unregister(void){
    if(pet_proc_entry){
        remove_proc_entry("tamagotchi_proc", NULL);
        pet_proc_entry = NULL;
    }
}
