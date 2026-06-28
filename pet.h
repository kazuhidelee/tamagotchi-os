#ifndef PET_H
#define PET_H

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

#define DEVICE_NAME "pet"
#define CLASS_NAME "tamagotchi"
#define LOG_SIZE 16
#define LOG_MSG_LEN 64

extern struct mutex pet_locak;



struct Pet {
    int hunger;
    int happiness;
    int health;
};

extern struct Pet pet;
extern struct mutex pet_lock;

const char *pet_get_mood(void);
void pet_clamp_state(void);

int pet_device_register(void);
void pet_device_unregister(void);

int pet_proc_register(void);
void pet_proc_unregister(void);

void pet_state_init(void);
void pet_timer_start(void);
void pet_timer_stop(void);


#endif