/* EC535 LAB4 mygpio */

#include <linux/init.h>  
#include <linux/module.h> 
#include <linux/kernel.h>         /* printk() */
#include <linux/slab.h>           /* kmalloc() */
#include <linux/fs.h>             /* everything... */
#include <linux/errno.h>          /* error codes */
#include <linux/types.h>          /* size_t */
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/fcntl.h>          /* O_ACCMODE */
#include <asm/system.h>           /* cli(), *_flags */
#include <asm/uaccess.h>          /* copy_from/to_user */ 
#include <linux/timer.h>          /* timer in kernel */
#include <linux/jiffies.h>        /* jiffies */
#include <linux/pid.h>            /* find_pid() */
#include <asm/arch/gpio.h>
#include <asm/hardware.h>
#include <asm/arch/i2c.h>
#include <asm/arch/pxa-regs.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <asm/arch/pxa-regs.h>
#include <asm-arm/arch/hardware.h>
#include <linux/io.h>
#include <linux/delay.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("mygpio Module");
MODULE_AUTHOR("ec535");

#define PWM_PRESCALE 	        __REG(0x40B00000)
#define PWM_DUTY 		__REG(0x40B00004)
#define PWM_PERIOD 		__REG(0x40B00008)
#define GPIO_DIR		__REG(0x40E0000C)
#define GPIO_S			__REG(0x40E00018)
#define GPIO_FUNC 		__REG(0x40E00058)


/******************************************************************/

/* Declaration of operation functions */
static ssize_t mygpio_read(    struct file *filp, 
                                char *buf, 
                                size_t count, 
                                loff_t *f_pos       );

static ssize_t mygpio_write(   struct file *filp, 
                                const char *buf, 
                                size_t count, 
                                loff_t *f_pos       );

static int mygpio_open(        struct inode *inode, 
                                struct file *filp   );

static int mygpio_release(     struct inode *inode, 
                                struct file *filp   );

/* init and exit the module */
static int mygpio_init(void);
static void mygpio_exit(void);

/* timerlist handler */
void timer_handler(unsigned long data);

void setCounterSpeed(char val);
//void setCounterValue_low_med_high(char val);

/* Structure that declares the usual file */
/* access functions */
struct file_operations mygpio_fops = {
    read: mygpio_read,
    write: mygpio_write,
    open: mygpio_open,
    release: mygpio_release,
};


/* Declaration of the init and exit functions */
module_init(mygpio_init);
module_exit(mygpio_exit); 

static unsigned capacity = 1024;
module_param(capacity, uint, S_IRUGO);

/* Global variables of the driver */
/* Major number */
static int mygpio_major = 61;

/* Buffer to store data */
static char *mygpio_buffer;

struct timer_list my_timer; 
static unsigned counter_period;    // counter_period / 4 seconds
static unsigned counter_value;   
static unsigned counter_direction; // STATES: COUNT_UP, COUNT_DOWN
static unsigned counter_state;     // STATES: RUNNING, STOPPED
static unsigned pwm_period = 3;    //this is the number of cycles - 1 that the period takes up
static unsigned pwm_prescale = 10; //this is the clock divider for the period time 
static unsigned pwm_duty_setting;  // used to hold pwm value of 1,2 and 4 (increasing brightness)

// These variables are used for handling double button press
#define DELAY       150
static unsigned time_btn0_pressed = 0;
static unsigned time_btn1_pressed = DELAY + 1;
static unsigned time_reset = 0;

// Read related items for building string 
static char speed;
static char* state;
static char* direction;
static char brightness;

// Counter variables
#define COUNT_UP    0
#define COUNT_DOWN  1
#define STOPPED     0
#define RUNNING     1
#define  COUNTER_SPEED_LOW 8
#define  COUNTER_SPEED_MED 4
#define  COUNTER_SPEED_HIGH 1

// Brightness variables for LED0
#define BRIGHTNESS_LOW         1
#define BRIGHTNESS_MED         2
#define BRIGHTNESS_HIGH        4

// define input buttons and output LEDs GPIO ports
#define BTN0        17
#define BTN1        101
#define BTN2        28
#define LED0        16  // 16 is used for PWM
#define LED1        29
#define LED2        30
#define LED3        31

#define DEBUG = 0;
/*******************************************************************/
//#define GPIO_DEBUG 1


void reset_counter()
{
  /* resets the counter to 0, while reseting the LED values as well */
  counter_value = 0;
  PWM_DUTY = 0 * pwm_duty_setting;
  pxa_gpio_set_value(LED1,0);
  pxa_gpio_set_value(LED2,0);
  pxa_gpio_set_value(LED3,0);

  // reset button time pressed values so they don't trigger reset again
  time_btn0_pressed = 0;
  time_btn1_pressed = DELAY + 1;


  /* Revert the states since they still flip for the original
   button pushes */
  if(counter_state == RUNNING){
    counter_state = STOPPED;
  }else{
    counter_state = RUNNING;
  }

  if(counter_direction == COUNT_UP){
    counter_direction = COUNT_DOWN;
  }else{
    counter_direction = COUNT_UP;
  }

}

irqreturn_t gpio_irq_btn0(int irq, void *dev_id, struct pt_regs *regs)
{

  //if(DEBUG) { printk("Button 0 pressed\n"); }
  time_btn0_pressed = jiffies;

  if(abs(time_btn0_pressed - time_btn1_pressed) < DELAY){
    reset_counter();
  }

  if(counter_state == RUNNING){
    counter_state = STOPPED;
  }else{
    counter_state = RUNNING;
  }


  return IRQ_HANDLED;
}

irqreturn_t gpio_irq_btn1(int irq, void *dev_id, struct pt_regs *regs)
{

  //if(DEBUG)printk("Button 1 pressed\n");
  
  time_btn1_pressed = jiffies;

  // Checking for reset
  if(abs(time_btn0_pressed - time_btn1_pressed) < DELAY){
    reset_counter();
  }


  if(counter_direction == COUNT_UP){
    counter_direction = COUNT_DOWN;
  }else{
    counter_direction = COUNT_UP;
  }
  return IRQ_HANDLED;
}


irqreturn_t gpio_irq_btn2(int irq, void *dev_id, struct pt_regs *regs)
{
  //no debounce, active for both edges
  //if(DEBUG) printk("Button 2 pressed\n");
  if(pwm_duty_setting == BRIGHTNESS_LOW){
    //if(DEBUG)printk("SET MED\n");
    pwm_duty_setting = BRIGHTNESS_MED;
  }else if(pwm_duty_setting == BRIGHTNESS_MED){
    //if(DEBUG) printk("SET HIGH\n");
    pwm_duty_setting = BRIGHTNESS_HIGH;
  }else if(pwm_duty_setting == BRIGHTNESS_HIGH){
    //if(DEBUG) printk("SET LOW\n");
    pwm_duty_setting = BRIGHTNESS_LOW;
  }
  
  int val0 = (counter_value & 1);
  PWM_DUTY = val0 * pwm_duty_setting;
  
  return IRQ_HANDLED;
}

static int mygpio_init(void)
{
    int result;

    /* Registering device */
    result = register_chrdev(mygpio_major, "mygpio", &mygpio_fops);
    if (result < 0)
    {
        printk(KERN_ALERT
            "mygpio: cannot obtain major number %d\n", mygpio_major);
        return result;
    }

    /* Allocating mygpio for the buffer */
    mygpio_buffer = (char *) kmalloc(capacity, GFP_KERNEL); 
    if (!mygpio_buffer)
    { 
        printk(KERN_ALERT "[kernel]-Insufficient kernel memory\n"); 
        result = -ENOMEM;
        goto fail; 
    } 
    memset( mygpio_buffer, 0, capacity);

    counter_period = COUNTER_SPEED_MED;
    counter_value = 0;
    counter_direction = COUNT_UP;
    counter_state = RUNNING; 

    /* initiate timer */
    init_timer(&my_timer);  
    my_timer.expires = jiffies + msecs_to_jiffies(250 * counter_period);  // set timer to 1000ms
    my_timer.function = timer_handler;
    my_timer.data = 0;  
    add_timer(&my_timer); 

    /* set up pwm - pwm_duty_setting is the variable that varies the brightness*/
    /* PWM_DUTY is the actual variable we set to make the brightness change*/
    pwm_duty_setting = BRIGHTNESS_LOW;
    pxa_set_cken(CKEN0_PWM0,1);

    PWM_PRESCALE = pwm_prescale;
    PWM_DUTY = pwm_duty_setting;
    PWM_PERIOD = pwm_period;
    GPIO_S = 0x00010000;
    GPIO_DIR = 0x00010000;
    GPIO_FUNC = 2;

    /* set up GPIO interface */
    gpio_direction_input(BTN0);
    gpio_direction_input(BTN1);
    //gpio_direction_output(LED0,0);
    gpio_direction_output(LED1,0);
    gpio_direction_output(LED2,0);
    gpio_direction_output(LED3,0);
#ifdef GPIO_DEBUG
    printk(KERN_ALERT"module created successfully.\n");
#endif

    /* Setup button 0 interrupt */
    pxa_gpio_mode(BTN0 | GPIO_IN);
    int irq_btn0 = IRQ_GPIO(BTN0);
    if (request_irq(irq_btn0, &gpio_irq_btn0, SA_INTERRUPT | SA_TRIGGER_RISING,"btn0", NULL) != 0 ) {
      printk ( "btn0_irq not acquired \n" );
      return -1;
    }else{
      printk ( "btn0_irq %d acquired successfully \n", irq_btn0 );
    }

    /* Setup button 1 interrupt */
    pxa_gpio_mode(BTN1 | GPIO_IN);
    int irq_btn1 = IRQ_GPIO(BTN1);
    if (request_irq(irq_btn1, &gpio_irq_btn1, SA_INTERRUPT | SA_TRIGGER_RISING,"btn1", NULL) != 0 ) {
      printk ( "btn1_irq not acquired \n" );
      return -1;
    }else{
      printk ( "btn1_irq %d acquired successfully \n", irq_btn1 );
    }

    /* Setup button 2 interrupt */
    pxa_gpio_mode(BTN2 | GPIO_IN);
    int irq_btn2 = IRQ_GPIO(BTN2);
    if (request_irq(irq_btn2, &gpio_irq_btn2, SA_INTERRUPT | SA_TRIGGER_RISING,"btn2", NULL) != 0 ) {
      printk ( "btn2_irq not acquired \n" );
      return -1;
    }else{
      printk ( "btn2_irq %d acquired successfully \n", irq_btn2 );
    }    


    // Setting up read variables
    state = (char *) kmalloc(5, GFP_KERNEL); 
    direction = (char *) kmalloc(5, GFP_KERNEL); 

    return 0;

fail: 
    mygpio_exit(); 
    return result;
}

static void mygpio_exit(void)
{
    /* Delete my timer */
    del_timer(&my_timer);

    /* Freeing the major number */
    unregister_chrdev(mygpio_major, "mygpio");

    /* Freeing buffer memory */
    if (mygpio_buffer)
    {
        kfree(mygpio_buffer);
    }

    free_irq(IRQ_GPIO(BTN0), NULL);
    free_irq(IRQ_GPIO(BTN1), NULL);
    free_irq(IRQ_GPIO(BTN2), NULL);
}



/************************* file operations ***************************/
static int mygpio_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int mygpio_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t mygpio_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{ 
    /* Prepare the buffer and write current node information to buffer */
    count  = capacity;

    // Set output values here
    // speed
    if(counter_period == COUNTER_SPEED_LOW){
      speed = 'L';
    }else if(counter_period == COUNTER_SPEED_MED){
      speed = 'M';
    }else{
      speed = 'H';
    }
    
    //state
    if(counter_state == RUNNING){
      strcpy(state, "Run\0");
    }else{
      strcpy(state,"Hold\0");
    }

    //direction
    if(counter_direction == COUNT_UP){
      strcpy(direction,"Up\0");
    }else{
      strcpy(direction,"Down\0");
    }
    
    //brightness
    if(pwm_duty_setting == 1){
      brightness = 'L';
    }else if(pwm_duty_setting == 2){
      brightness = 'M';
    }else if(pwm_duty_setting == 4){
      brightness = 'H';
    }

    // Create string for transferring to be read
    if(*f_pos == 0){
        memset(mygpio_buffer, 0, capacity);
	        sprintf(mygpio_buffer,
			"%d  %c  %s  %s  %c\n",counter_value,speed,state,direction,brightness);
    } else return 0;


    /* Transfering data to user space */ 
    if (copy_to_user(buf, mygpio_buffer, count))
    {
        return -EFAULT;
    }
    *f_pos = count; 
    return count; 
}


static ssize_t mygpio_write(struct file *filp, const char *buf,
                            size_t count, loff_t *f_pos)
{

    memset(mygpio_buffer, 0, capacity);
    if (copy_from_user(mygpio_buffer, buf, count))
    {
        return -EFAULT;
    }

    switch(mygpio_buffer[0]){
        case 'f': if  ( ( mygpio_buffer[1] - '0' >= 0 ) && (mygpio_buffer[1] - '0' <= 9) && strlen(mygpio_buffer) == 3)
                        counter_period = mygpio_buffer[1] - '0';
                  else printk(KERN_ALERT "input must be 0-9.\n");
                  break;
                  

        case 'v':  setCounterSpeed(mygpio_buffer[1]); break;

        default : printk(KERN_ALERT"f[num]: num range from 0 to 9\nv[hex] hex range from 0 to f\n"); break;
                  
    }

    return count;
}



void setCounterSpeed(char val)
{
    if ( val == 'l' || val == 'L' ){
      counter_period = COUNTER_SPEED_LOW;
      return;
    }else if ( val == 'm' || val == 'M' ){
      counter_period = COUNTER_SPEED_MED;
      return;
    } else if (val == 'h' || val == 'H') { 
      counter_period = COUNTER_SPEED_HIGH;
      return;
    }else{
        printk(KERN_ALERT"value inputs: v[l,m,h] (for low, medium, high, respectively\n");
    }  

}


void timer_handler(unsigned long data)  
{   
    int val0,val1,val2,val3;
    /* when old timer expired, add a new timer */
    my_timer.expires = jiffies + msecs_to_jiffies(250 * counter_period); 
    my_timer.function = timer_handler;
    my_timer.data = 0;  
    add_timer(&my_timer);


    if (counter_state == RUNNING){
        if (counter_direction == COUNT_UP){
            counter_value += 1;
            if (counter_value == 16)
                counter_value = 0;
        } else if (counter_direction == COUNT_DOWN){
            if (counter_value == 0) counter_value = 16;
            counter_value -= 1;
        }
    }

    val0 = (counter_value & 1);
    val1 = (counter_value & 3)  >> 1;
    val2 = (counter_value & 7)  >> 2;
    val3 = (counter_value & 15) >> 3;
    //printk("BRIGHTNESS: %d\n", pwm_duty_setting);
    //pxa_gpio_set_value(LED0,val0);
    PWM_DUTY = val0 * pwm_duty_setting;
    pxa_gpio_set_value(LED1,val1);
    pxa_gpio_set_value(LED2,val2);
    pxa_gpio_set_value(LED3,val3);

#ifdef GPIO_DEBUG
    printk("LEDs 0: %d\nLEDs 1: %d\nLEDs 2: %d\nLEDs 3: %d\n",val0,val1,val2,val3);
    printk("Counter Value: %u \nCounter Period: %u / 4 second(s)\nCounter Dire
ction: %u (0-UP,1-DOWN)\nCounter State: %u (0-STOP,1-RUNNING)\n",
                counter_value,counter_period,counter_direction,counter_state);
    printk("BTN0: %u \n BTN1: %u\n",pxa_gpio_get_value(BTN0),pxa_gpio_get_value(BTN1));
#endif

}  
