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
#include <linux/string.h>
#include "pet.h"

#define PET_LOG_CAPACITY 64
#define PET_LOG_MSG_SIZE 128

static DEFINE_MUTEX(pet_log_lock);

struct pet_log_entry {
    unsigned long timestamp_jiffies;
    char message[PET_LOG_MSG_SIZE];
};

struct pet_log {
    struct pet_log_entry entries[PET_LOG_CAPACITY];
    unsigned int head; // basically next index to write
    unsigned int count; // number of valid entries, max 64
};

static struct pet_log pet_log;

void pet_log_event(const char *event)
{
    struct pet_log_entry *entry =  &pet_log.entries[pet_log.head];
    // TODO:
    // copy event into event_log[log_index]
    // advance log_index circularly
    // increase log_count until LOG_SIZE
    mutex_lock(&pet_log_lock);
    entry->timestamp_jiffies = jiffies;
    strscpy(entry->message, event, PET_LOG_MSG_SIZE);
    pet_log.head = (pet_log.head + 1) % PET_LOG_CAPACITY;
    if(pet_log.count < PET_LOG_CAPACITY){
        pet_log.count++;
    }
    mutex_unlock(&pet_log_lock);
}

void pet_log_show(struct seq_file *m){
    int starting;
    int index;
    int i;
    struct pet_log_entry *curr_entry;
    mutex_lock(&pet_log_lock);
    if (pet_log.count == PET_LOG_CAPACITY){
        starting = pet_log.head;
    }else if (pet_log.count == 0){
        seq_printf(m, "no log entries yet");
        return;
    }else{
        starting = 0;
    }

    for(i = 0; i < pet_log.count; ++i){
        index = (starting + i) % PET_LOG_CAPACITY;
        curr_entry = &pet_log.entries[index];
        seq_printf(m, "[%lu] %s\n", curr_entry->timestamp_jiffies, curr_entry->message);
    }
    mutex_unlock(&pet_log_lock);
}