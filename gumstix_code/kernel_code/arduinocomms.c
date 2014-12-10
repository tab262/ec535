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
#include <asm-arm/arch/hardware.h> /* macros and gpio interupts */
#include <linux/interrupt.h>  /* gpio interupts */
#include <linux/irq.h>        /* set_irq_type */

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Jonathan Hirokawa");

//========================================================================
//Global Variables
//========================================================================
static int arduino_comms_major = 61;         //major number for registering this module
struct fasync_struct *async_queue;           //for async reading
static struct proc_dir_entry *proc_entry;    //obj that holds the proc entry

int arduino_yaw = 0;              //these vars hold the arduino values
int arduino_pitch = 0;            //ie. what the kernel thinks the arduino
int arduino_throttle = 0;         //has currently

int target_yaw = 0;           //these values hold the target control params
int target_pitch = 0;         //that we're trying to get to
int target_throttle = 0;

#define MANUAL 'm'            //specifier for manual control
#define AUTO 'a'              //specifier for auto pilot

#define INC 1                 //increase control value
#define DEC 0                 //decrease control value
#define MULT5 1               //increment/decrement by 5
#define MULT1 0               //increment/decrement by 1

#define THROTTLE_INCDEC 29 //pin for throttle: use with INC or DEC
#define THROTTLE_RATE 30 //pin for throttle rate: use with MULT5 or MULT1

#define YAW_INCDEC 113 //101 //pin for yaw: use with INC or DEC
#define YAW_RATE 28 //17 //pin for yaw rate: use with MULT5 or MULT1

#define PITCH_INCDEC 46 //101 //113 //pin for pitch: use with INC or DEC
#define PITCH_RATE 47 //17 //28 //pin for pitch rate: use with MULT5 or MULT1

#define ARDUINO_CMD_REQ 31 //pin used by arduino to request new command

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
//irqreturn_t request_cmd_cb(int irq, void *dev_id, struct pt_regs *regs);
void transmit(int new_pitch, int new_yaw, int new_throttle);
void simple_transmit(int new_pitch, int new_yaw, int new_throttle);

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

irqreturn_t request_cmd_cb(int irq, void *dev_id, struct pt_regs *regs)
{
     transmit(target_yaw, target_pitch, target_throttle);
     return IRQ_HANDLED; //handler was correctly invoked and delt with
}



/* The init function called when module is installed */
static int arduino_comms_init(void){
     int result;         //temp status var
     int cmd_req_irq;    //the irq number associated with the gpio number

     /* Attempt to register the device */
     result = register_chrdev(arduino_comms_major, "arduino_comms", &arduino_comms_fops);
     //check to see if registration was successful
     if(result < 0){
          printk(KERN_ALERT "arduino_comms:  cannot obtain major number %d\n", arduino_comms_major);
          return result;
     }

     proc_entry = create_proc_entry("arduino_comms", 0644, NULL);
     if(proc_entry == NULL){
          printk(KERN_ALERT "Couldn't create proc entry\n");
          result = -ENOMEM;
     }else{
          //proc_entry->write_proc = write_2_proc;
          proc_entry->read_proc = arduino_comms_proc_read;
          proc_entry->owner = THIS_MODULE;
     }
     
     //IO pin setup
     gpio_direction_output(THROTTLE_INCDEC,0);//set pin "THROTTLE_INCDEC" as output w/ init val of 0
     gpio_direction_output(THROTTLE_RATE,0);  //set pin "THROTTLE_RATE" as output w/ init val of 0

     
     gpio_direction_output(YAW_INCDEC, 0);
     gpio_direction_output(YAW_RATE, 0);
     
     
     gpio_direction_output(PITCH_INCDEC, 0);
     gpio_direction_output(PITCH_RATE, 0);
     
     //gpio_direction_input(ARDUINO_CMD_REQ);  //set pin used by arduino to request new command as input
     pxa_gpio_mode(ARDUINO_CMD_REQ | GPIO_IN);
     cmd_req_irq = IRQ_GPIO(ARDUINO_CMD_REQ);//get the irq number corresponding to the gpio_number
     set_irq_type(cmd_req_irq, IRQT_RISING); //interupt triggered on rising edge (0 to 1 signal transition)
     //register the gpio interrupt
     //SA_INTERRUPT | SA_TRIGGER_RISING
     if(request_irq(cmd_req_irq, &request_cmd_cb, 0, "ARDUINO_CMD_REQ", NULL) != 0){
          printk(KERN_ALERT "Interupt for Arduino command requests not aquired\n");
          goto fail;
     }

     printk("arduino_comms loaded.\n");

     return 0;

     fail:
          arduino_comms_exit();
          return result;
}

//------------------------------------------------------------------------
/* The exit function (called when module is removed) */
static void arduino_comms_exit(void){
     unregister_chrdev(arduino_comms_major, "arduino_comms");
     free_irq(IRQ_GPIO(ARDUINO_CMD_REQ), NULL);
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

     memset(msgptr, 0, msg_len); //clean out buffer

     //do a few basic checks to avoid errors
     if(*f_pos >= msg_len) return -ENOSPC;   //reached end of buffer
     if(count > msg_len - *f_pos) count = msg_len - *f_pos;   //don't go over end

     //bring from user space to kernel space
     if(copy_from_user(msgptr + *f_pos, buf, count)) return -EFAULT;

     //printk(KERN_INFO "MESSAGE:  %s\n", message);

     //parse the message
     sscanf(message, "%c,%d,%d,%d\n", &mode, &target_yaw,
               &target_pitch, &target_throttle);

     /*
     printk(KERN_INFO "MODE:  %c\n", mode);
     printk(KERN_INFO "YAW:  %d\n", target_yaw);
     printk(KERN_INFO "PITCH:  %d\n", target_pitch);
     printk(KERN_INFO "THROTTLE:  %d\n", target_throttle);
     */
     if(mode == MANUAL){
       //printk(KERN_INFO "MANUAL CONTROL\n");
       //transmit(target_yaw, target_pitch, target_throttle);
     }else if(mode == AUTO){
	  simple_transmit(target_yaw, target_pitch, target_throttle);
     }else{
       //printk(KERN_INFO "ERROR:  MODE SPECIFIER NOT RECOGNIZED\n");
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
/* Helper function that transmits the commands via the GPIO pins as needed*/
void transmit(int new_pitch, int new_yaw, int new_throttle)
{
     int diff_yaw, diff_pitch, diff_throttle;

     //changes in yaw
     diff_yaw = new_yaw - arduino_yaw;
     if(diff_yaw >=  0){
       gpio_set_value(YAW_INCDEC, INC);
          if(diff_yaw >= 5){
	    gpio_set_value(YAW_RATE, MULT5);
               arduino_yaw += 5;
          }else{
	    gpio_set_value(YAW_RATE, MULT1);
               arduino_yaw += 1;
          }
     }else if(diff_yaw < 0){
         gpio_set_value(YAW_INCDEC, DEC);
          if(diff_yaw <= -5){
	             gpio_set_value(YAW_RATE, MULT5);
               arduino_yaw -= 5;
          }else{
	    gpio_set_value(YAW_RATE, MULT1);
               arduino_yaw--;
          }
     }

     //changes in pitch
     diff_pitch = new_pitch - arduino_pitch;
     
     if(diff_pitch >=  0){
       gpio_set_value(PITCH_INCDEC, INC);
          if(diff_pitch >= 5){
	    gpio_set_value(PITCH_RATE, MULT5);
            arduino_pitch += 5;
          }else{
	    gpio_set_value(PITCH_RATE, MULT1);
               arduino_pitch += 1;
          }
     }else if(diff_pitch < 0){
      gpio_set_value(PITCH_INCDEC, DEC);
          if(diff_pitch <= -5){
	    gpio_set_value(PITCH_RATE, MULT5);
               arduino_pitch -= 5;
          }else{
	    gpio_set_value(PITCH_RATE, MULT1);
               arduino_pitch--;
          }
     }
     

     //changes in throttle
     diff_throttle = new_throttle - arduino_throttle;
     if(diff_throttle >=  0){
       gpio_set_value(THROTTLE_INCDEC, INC);
          if(diff_throttle >= 5){
	       gpio_set_value(THROTTLE_RATE, MULT5);
               arduino_throttle += 5;
          }else{
	    gpio_set_value(THROTTLE_RATE, MULT1);
	    arduino_throttle += 1;
          }
     }else if(diff_throttle < 0){
       gpio_set_value(THROTTLE_INCDEC, DEC);
          if(diff_throttle <= -5){
	    gpio_set_value(THROTTLE_RATE, MULT5);
	    arduino_throttle -= 5;
          }else{
	    gpio_set_value(THROTTLE_RATE, MULT1);
	    arduino_throttle--;
          }
     }
     //printk(KERN_INFO "YAW: %d\t PITCH:  %d\t THROTTLE:  %d\n",
     //	    arduino_yaw, arduino_pitch, arduino_throttle);
}

//------------------------------------------------------------------------
/* Callback function for when the ARUINO_CMD_REG pin goes high*/


void simple_transmit(int new_pitch, int new_yaw, int new_throttle)
{
  //printk("SIMPLE TRANSMIT\n");
  
  


  if(new_throttle){
    gpio_set_value(THROTTLE_RATE, MULT1);
    gpio_set_value(THROTTLE_INCDEC, INC);
  }else{
    gpio_set_value(THROTTLE_RATE, MULT5);
    gpio_set_value(THROTTLE_INCDEC, DEC);
  }




  gpio_set_value(YAW_RATE, MULT5); 
  if(new_yaw){
    gpio_set_value(YAW_INCDEC, INC);
  }else{
    gpio_set_value(YAW_INCDEC, DEC);
  }
  

  int diff_pitch = new_pitch - arduino_pitch;
     
  if(diff_pitch >=  0){
    gpio_set_value(PITCH_INCDEC, INC);
    if(diff_pitch >= 5){
      gpio_set_value(PITCH_RATE, MULT5);
      arduino_pitch += 5;
    }else{
      gpio_set_value(PITCH_RATE, MULT1);
      arduino_pitch += 1;
    }
  }else if(diff_pitch < 0){
    gpio_set_value(PITCH_INCDEC, DEC);
    if(diff_pitch <= -5){
      gpio_set_value(PITCH_RATE, MULT5);
      arduino_pitch -= 5;
    }else{
      gpio_set_value(PITCH_RATE, MULT1);
      arduino_pitch--;
    }
  }
}
