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
static char event_log[LOG_SIZE][LOG_MSG_LEN];
static int log_index = 0;
static int log_count = 0;


static void pet_log_event(const char *event)
{
    // TODO:
    // copy event into event_log[log_index]
    // advance log_index circularly
    // increase log_count until LOG_SIZE
}