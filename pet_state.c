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

static struct timer_list pet_timer;

const char *pet_get_mood(void){
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


void pet_clamp_state(void)
{
    if (pet.hunger < 0) pet.hunger = 0;
    if (pet.hunger > 100) pet.hunger = 100;
    if (pet.happiness < 0) pet.happiness = 0;
    if (pet.happiness > 100) pet.happiness = 100;
    if (pet.health < 0) pet.health = 0;
    if (pet.health > 100) pet.health = 100;
}

static void pet_tick(struct timer_list *t)
{
    mutex_lock(&pet_lock);
    // TODO:
    // hunger should increase over time
    // happiness should decrease over time
    // if hunger gets too high, health should decrease
    // clamp values between 0 and 100
    pet.hunger += 1;
    pet.happiness -= 1;
    if(pet.hunger > 80){
        pet.health -= 1;
    }
    pet_clamp_state();
    
    printk(KERN_INFO "pet: tick\n");
    mutex_unlock(&pet_lock);
    pet_log_event("timer tick");
    mod_timer(&pet_timer, jiffies + msecs_to_jiffies(5000));
}

void pet_state_init(void)
{
    pet.health = 100;
    pet.hunger = 50;
    pet.happiness = 50;
}

void pet_timer_start(void)
{
    timer_setup(&pet_timer, pet_tick, 0);
    mod_timer(&pet_timer, jiffies + msecs_to_jiffies(5000));
}

void pet_timer_stop(void)
{
    timer_delete_sync(&pet_timer);
}


// pet_apply_command()