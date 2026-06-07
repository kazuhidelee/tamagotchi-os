#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/timer.h>

#define DEVICE_NAME "pet"
#define CLASS_NAME "tamagotchi"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tony");
MODULE_DESCRIPTION("Tamagotchi Linux character device");

static int major_number;
static struct class* pet_class = NULL;
static struct device* pet_device = NULL;
static struct timer_list pet_timer;
static int hunger = 50;
static int happiness = 50;
static int health = 100;

static ssize_t pet_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    char msg[128];
    int msg_len;
    const char *mood;

    if(health == 0){
        mood = "Dead";
    }else{
        if(hunger >= 50){
            mood = "Starving";
        }else{
            if(happiness <= 40){
                mood = "Sad";
            }else if(happiness >= 60){
                mood = "Happy";
            }else{
                mood = "normal";
            }
        }
    }
    msg_len = snprintf(msg, sizeof(msg),
        "Name: Mochi\nHealth: %d\nHunger: %d\nHappiness: %d\nMood: %s\n",
        health, hunger, happiness, mood
    );

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

    command[len] = '\0';

    if (strncmp(command, "feed", 4) == 0) {
        hunger -= 10;
        happiness += 2;
    } else if (strncmp(command, "play", 4) == 0) {
        happiness += 10;
        hunger += 5;
    } else if (strncmp(command, "sleep", 5) == 0) {
        health += 5;
        hunger += 3;
    } else if (strncmp(command, "medicine", 8) == 0){
        health += 10;
    }

    if (hunger < 0) hunger = 0;
    if (hunger > 100) hunger = 100;
    if (happiness < 0) happiness = 0;
    if (happiness > 100) happiness = 100;
    if (health < 0) health = 0;
    if (health > 100) health = 100;

    return len;
}

static struct file_operations fops = {
    .read = pet_read,
    .write = pet_write,
};

static void pet_tick(struct timer_list *t)
{
    // TODO:
    // hunger should increase over time
    // happiness should decrease over time
    // if hunger gets too high, health should decrease
    // clamp values between 0 and 100
    if(hunger < 100){
        hunger += 1;
    }
    if(happiness > 0){
        happiness -= 1;
    }
    if(hunger > 80){
        if(health > 0) health -= 1;
    }
    
    printk(KERN_INFO "pet: tick\n");

    mod_timer(&pet_timer, jiffies + msecs_to_jiffies(5000));
}

static int __init pet_init(void) {
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
    timer_setup(&pet_timer, pet_tick, 0);
    mod_timer(&pet_timer, jiffies + msecs_to_jiffies(5000));

    printk(KERN_INFO "pet: /dev/pet created\n");
    return 0;
}

static void __exit pet_exit(void) {
    timer_delete_sync(&pet_timer);
    device_destroy(pet_class, MKDEV(major_number, 0));
    class_destroy(pet_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "pet: /dev/pet removed\n");
}

module_init(pet_init);
module_exit(pet_exit);
