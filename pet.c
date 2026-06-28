#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mutex.h>

#define DEVICE_NAME "pet"
#define CLASS_NAME "tamagotchi"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tony");
MODULE_DESCRIPTION("Tamagotchi Linux character device");

DEFINE_MUTEX(pet_lock);

static int major_number;
static struct class* pet_class = NULL;
static struct device* pet_device = NULL;
static struct timer_list pet_timer;
struct Pet{
    int hunger;
    int happiness;
    int health;
};
static struct Pet pet;


static const char *pet_get_mood(void){
    const char* mood;
    if(pet.health == 0){
        mood = "Dead";
    }else{
        if(pet.hunger >= 70){
            mood = "Starving";
        }else{
            if(pet.happiness <= 40){
                mood = "Sad";
            }else if(pet.happiness >= 60){
                mood = "Happy";
            }else{
                mood = "normal";
            }
        }
    }
    return mood;
}

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

    if (pet.hunger < 0) pet.hunger = 0;
    if (pet.hunger > 100) pet.hunger = 100;
    if (pet.happiness < 0) pet.happiness = 0;
    if (pet.happiness > 100) pet.happiness = 100;
    if (pet.health < 0) pet.health = 0;
    if (pet.health > 100) pet.health = 100;
    mutex_unlock(&pet_lock);
    return len;
}

static struct file_operations fops = {
    .read = pet_read,
    .write = pet_write,
};

static void pet_tick(struct timer_list *t)
{
    mutex_lock(&pet_lock);
    // TODO:
    // hunger should increase over time
    // happiness should decrease over time
    // if hunger gets too high, health should decrease
    // clamp values between 0 and 100
    if(pet.hunger < 100){
        pet.hunger += 1;
    }
    if(pet.happiness > 0){
        pet.happiness -= 1;
    }
    if(pet.hunger > 80){
        if(pet.health > 0) pet.health -= 1;
    }
    
    printk(KERN_INFO "pet: tick\n");
    mutex_unlock(&pet_lock);
    mod_timer(&pet_timer, jiffies + msecs_to_jiffies(5000));
}


static int __init pet_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    pet.health = 100;
    pet.hunger = 50;
    pet.happiness = 50;
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
    timer_setup(&pet_timer, pet_tick, 0);
    // Arguments: File name, Permissions (0 means default), Parent dir, Proc Ops
    proc_create("tamagotchi_proc", 0, NULL, &my_proc_fops);
    mod_timer(&pet_timer, jiffies + msecs_to_jiffies(5000));
    printk(KERN_INFO "pet: /dev/pet created\n");
    return 0;
}



static void __exit pet_exit(void) {
    timer_delete_sync(&pet_timer);
    device_destroy(pet_class, MKDEV(major_number, 0));
    class_destroy(pet_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    remove_proc_entry("tamagotchi_proc", NULL);
    printk(KERN_INFO "pet: /dev/pet removed\n");
    
}

module_init(pet_init);
module_exit(pet_exit);
