/*Include the needed device drivers*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>     /* printk() */
#include <linux/slab.h>       /* kmalloc() */
#include <linux/fs.h>         /* everything... */
#include <linux/errno.h>      /* error codes */
#include <linux/types.h>      /* size_t */
#include <linux/proc_fs.h>    /* proc file system */
#include <linux/fcntl.h>      /* O_ACCMODE not sure what this is */
#include <asm/uaccess.h>      /* copy from/to user */
#include <asm/gpio.h>         /* gpio functionality */

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Jonathan Hirokawa");

//========================================================================
//Global Variables
//========================================================================
static int arduino_comms_major = 61;         //major number for registering this module
struct fasync_struct *async_queue;      //for async reading
static struct proc_dir_entry *proc_entry;  //obj that holds the proc entry

int old_yaw;                  //these vars hold the old values
int old_pitch;                //ie. what the kernel thinks the arduino
int old_throttle;             //has currently
int old_trim;

#define YAW 0                 //these define the number/code associated
#define PITCH 1               //with each parameter we need to send
#define THROTTLE 2            //(the bit code)
#define TRIM 3                //ex. YAW -> both param pins off

#define MANUAL 'm'            //specifier for manual control
#define AUTO 'a'              //specifier for auto pilot

#define INC 1                 //set INCDEC to this to increase
#define DEC 0                 //set INCDEC to this to decrease

#define CMD_BIT0 16             //GPIO pin for first bit of param identifier
#define CMD_BIT1 18             //GPIO pin for 2nd bit of param identifier
#define INCDEC 20             //GPIO pin for incrementing or decrementing param

#define DEBUG 0
//========================================================================
//Function Declarations
//========================================================================
static int arduino_comms_open(struct inode *inode, struct file *filp);
static int arduino_comms_release(struct inode *inode, struct file *filp);
static ssize_t arduino_comms_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static ssize_t arduino_comms_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
static int arduino_comms_init(void);
static void arduino_comms_exit(void);
static ssize_t arduino_comms_proc_read(char *page, char **page_location, off_t offset, int page_length, int *eof, void *data);
//static ssize_t write_2_proc(struct file *filp, const char __user *buff, unsigned long len, void *data);
void set_cmd(int cmd);
void transmit(int new_pitch, int new_yaw, int new_throttle, int new_trim);

/* Declaration of init and exit functions */
module_init(arduino_comms_init);
module_exit(arduino_comms_exit);

/* File I/O struct */
struct file_operations arduino_comms_fops = {
write: arduino_comms_write,
       read: arduino_comms_read,
       open: arduino_comms_open,
       release: arduino_comms_release,
};

//========================================================================
//Function Definitions
//========================================================================
/* The init function called when module is installed */
static int arduino_comms_init(void){
     int result;    //temp status var

     /* Attempt to register the device */
     result = register_chrdev(arduino_comms_major, "arduino_comms", &arduino_comms_fops);
     //check to see if registration was successful
     if(result < 0){
          printk(KERN_ALERT "arduino_comms:  cannot obtain major number %d\n", arduino_comms_major);
          return result;
     }

     proc_entry = create_proc_entry("arduino_comms", 0644, NULL);
     if(proc_entry ==NULL){
          printk(KERN_ALERT "Couldn't create proc entry\n");
          result = -ENOMEM;
     }else{
          //proc_entry->write_proc = write_2_proc;
          proc_entry->read_proc = arduino_comms_proc_read;
          proc_entry->owner = THIS_MODULE;
     }

     //IO pin setup
     gpio_direction_output(CMD_BIT0,0);//set pin "CMD_BIT0" as output w/ init val of 0
     gpio_direction_output(CMD_BIT1,0);//set pin "CMD_BIT1" as output w/ init val of 0
     gpio_direction_output(INCDEC,0);//set pin "INCDEC" as output w/ init val of 0

     printk("arduino_comms loaded.\n");

     return 0;
}

//------------------------------------------------------------------------
/* The exit function (called when module is removed) */
static void arduino_comms_exit(void){
     unregister_chrdev(arduino_comms_major, "arduino_comms");
     remove_proc_entry("arduino_comms", &proc_root);
     printk(KERN_ALERT "Removing arduino_comms module\n");
}

//------------------------------------------------------------------------
/* The open function is the first operation performed on the dev file */
static int arduino_comms_open(struct inode *inode, struct file *filp){
     return 0;
}

//------------------------------------------------------------------------
/* The release (close) function is the opposite of open */
static int arduino_comms_release(struct inode *inode, struct file *filp){
     return 0;
}
//------------------------------------------------------------------------
/* The read function is called when someone tries to read fro /dev/arduino_comms */
static ssize_t arduino_comms_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
     return count;
}
//------------------------------------------------------------------------
/* The write function is called when someone writes to /dev/arduino_comms */
//INPUTS
//   filp  = file pointer struct
//   buf   = user-space memory buffer
//   count = number of bytes incoming from user-space
//   f_pos = file position index
static ssize_t arduino_comms_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
     int msg_len = 128;       //max length of message we're expecting
     char message[msg_len];   //will hold local version of incoming message
     char *msgptr = message;  //pointer to message
     char mode = '0';         //auto pilot or manual
     int new_yaw = -1;            //turning left or right
     int new_pitch = -1;          //tipping up or down
     int new_throttle = -1;       //power to the propellers
     int new_trim = -1;           //adjustment to prevent unwated rotation

     memset(msgptr, 0, msg_len); //clean out buffer

     //do a few basic checks to avoid errors
     if(*f_pos >= msg_len) return -ENOSPC;   //reached end of buffer
     if(count > msg_len - *f_pos) count = msg_len - *f_pos;   //don't go over end

     //bring from user space to kernel space
     if(copy_from_user(msgptr + *f_pos, buf, count)) return -EFAULT;

     printk(KERN_INFO "MESSAGE:  %s\n", message);

     //parse the message
     sscanf(message, "%c,%d,%d,%d,%d", &mode, &new_yaw,
               &new_pitch, &new_throttle, &new_trim);

     if(mode == MANUAL){
          printk(KERN_INFO "MANUAL CONTROL\n");
          transmit(new_yaw, new_pitch, new_throttle, new_trim);

     }else if(mode == AUTO){
          printk(KERN_INFO "WARNING:  AUTOPILOT NOT YET IMPLEMENTED\n");
     }else{
          printk(KERN_INFO "ERROR:  MODE SPECIFIER NOT RECOGNIZED\n");
     }
     return count;
}

//------------------------------------------------------------------------
/* proc_read gets called when someone attempts to read from the proc file */
static ssize_t arduino_comms_proc_read(char *page, char **page_location, off_t offset,
          int page_length, int *eof, void *data)
{

     return 0;
}

//------------------------------------------------------------------------
/* Helper function to set pins specifying what command we're sending */
void set_cmd(int cmd)
{
     if(cmd == YAW){
          gpio_set_value(CMD_BIT0, 0);
          gpio_set_value(CMD_BIT1, 0);
     }else if(cmd == PITCH){
          gpio_set_value(CMD_BIT0, 0);
          gpio_set_value(CMD_BIT1, 1);
     }else if(cmd== THROTTLE){
          gpio_set_value(CMD_BIT0, 1);
          gpio_set_value(CMD_BIT1, 0);
     }else if(cmd == TRIM){
          gpio_set_value(CMD_BIT0, 1);
          gpio_set_value(CMD_BIT1, 1);
     }else{
          printk(KERN_INFO "ERROR:  CMD not recognized\n");
     }
}

//------------------------------------------------------------------------
/* Helper function that transmits the commands via the GPIO pins as needed*/
void transmit(int new_pitch, int new_yaw, int new_throttle, int new_trim)
{

     //changes in yaw
     if(new_yaw > old_yaw){
          set_cmd(YAW);
          gpio_set_value(INCDEC, INC);
     }else if(new_yaw < old_yaw){
          set_cmd(YAW);
          gpio_set_value(INCDEC, DEC);
     }

     //changes in pitch
     if(new_pitch > old_pitch){
          set_cmd(PITCH);
          gpio_set_value(INCDEC, INC);
     }else if(new_pitch < old_pitch){
          set_cmd(PITCH);
          gpio_set_value(INCDEC, DEC);
     }

     //changes in throttle
     if(new_throttle > old_throttle){
          set_cmd(THROTTLE);
          gpio_set_value(INCDEC, INC);
     }else if(new_throttle < old_throttle){
          set_cmd(THROTTLE);
          gpio_set_value(INCDEC, DEC);
     }

     //changes in trim
     if(new_trim > old_trim){
          set_cmd(TRIM);
          gpio_set_value(INCDEC, INC);
     }else if(new_trim < old_trim){
          set_cmd(TRIM);
          gpio_set_value(INCDEC, DEC);
     }
}

