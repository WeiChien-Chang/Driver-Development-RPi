#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h> //copy_to/from_user()
#include <linux/gpio.h>     //GPIO

//LED is connected to this GPIO
#define GPIO_18 (18)
#define GPIO_24 (24)
#define GPIO_25 (25)
#define GPIO_12 (12)
#define GPIO_16 (16)
#define GPIO_20 (20)
#define GPIO_21 (21)

#define NUM_LEDS 7  // 定義 LED 的數量

static int gpio_pins[NUM_LEDS] = {18, 24, 25, 12, 16, 20, 21};  // 使用的 GPIO 引腳號

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);

static int etx_release(struct inode *inode, struct file *file);

static ssize_t etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);

static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
/******************************************************/

//File operation structure
static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .read = etx_read,
    .write = etx_write,
    .open = etx_open,
    .release = etx_release,
};

/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    uint8_t gpio_states[7];  // 用來存放7個GPIO引腳的狀態
    
    //reading GPIO value
    gpio_states[0] = gpio_get_value(GPIO_18);
    gpio_states[1] = gpio_get_value(GPIO_24);
    gpio_states[2] = gpio_get_value(GPIO_25);
    gpio_states[3] = gpio_get_value(GPIO_12);
    gpio_states[4] = gpio_get_value(GPIO_16);
    gpio_states[5] = gpio_get_value(GPIO_20);
    gpio_states[6] = gpio_get_value(GPIO_21);

    //write to user
    if (copy_to_user(buf, gpio_states, sizeof(gpio_states)) > 0) {
        pr_err("ERROR: Not all the bytes have been copied to user\n");
        return -EFAULT;  // 返回錯誤碼，表示拷貝失敗
    }

    pr_info("Read function: GPIO_18 = %d, GPIO_24 = %d, GPIO_25 = %d, GPIO_12 = %d, GPIO_16 = %d, GPIO_20 = %d, GPIO_21 = %d\n",
            gpio_states[0], gpio_states[1], gpio_states[2], gpio_states[3], gpio_states[4], gpio_states[5], gpio_states[6]);
    
    return sizeof(gpio_states);  // 返回傳送給用戶的字節數
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    uint8_t rec_buf[10] = {0};
    if( copy_from_user( rec_buf, buf, len ) > 0) {
        pr_err("ERROR: Not all the bytes have been copied from user\n");
    }

    pr_info("Write Function : GPIO_21 Set = %c\n", rec_buf[0]);

    static const unsigned char digit_map[10][NUM_LEDS] = {
        /* 0 */ {1,1,1,1,1,1,0},
        /* 1 */ {0,0,0,0,1,1,0},
        /* 2 */ {1,1,0,1,1,0,1},
        /* 3 */ {1,0,0,1,1,1,1},
        /* 4 */ {1,0,1,0,1,1,0},
        /* 5 */ {1,0,1,1,0,1,1},
        /* 6 */ {1,1,1,1,1,0,1},
        /* 7 */ {0,0,0,0,1,1,1},
        /* 8 */ {1,1,1,1,1,1,1},
        /* 9 */ {1,0,1,1,1,1,1},
    };

    if (rec_buf[0] >= '0' && rec_buf[0] <= '9') {
        int d = rec_buf[0] - '0';
        int i;
        for (i = 0; i < NUM_LEDS; ++i) {
            gpio_set_value(gpio_pins[i], digit_map[d][i]);
        }
    }
    else 
    {
        pr_err("Unknown command : Please provide either 1 or 0 \n");
    }

    return len;
}

/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0)
    {
        pr_err("Cannot allocate major number\n");
        goto r_unreg;
    }

    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&etx_cdev,&fops);

    /*Adding character device to the system*/
    if((cdev_add(&etx_cdev,dev,1)) < 0)
    {
        pr_err("Cannot add the device to the system\n");
        goto r_del;
    }

    /*Creating struct class*/
    if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL)
    {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL)
    {
        pr_err( "Cannot create the Device \n");
    goto r_device;
    }

    //Checking the GPIO is valid or not
    for (int i = 0; i < NUM_LEDS; i++) 
    {
        if (gpio_is_valid(gpio_pins[i]) == false) 
        {
            pr_err("GPIO %d is not valid\n", gpio_pins[i]);
            goto r_device;
        }
    }

    //Requesting the GPIO
    for (int i = 0; i < NUM_LEDS; i++) 
    {
        if (gpio_request(gpio_pins[i], "GPIO_LED") < 0) 
        {
            pr_err("ERROR: GPIO %d request failed\n", gpio_pins[i]);
            goto r_gpio;
        }
    }

    //configure the GPIO as output
    for (int i = 0; i < NUM_LEDS; i++) 
    {
        gpio_direction_output(gpio_pins[i], 0);  
    }

    /* Using this call the GPIO 21 will be visible in /sys/class/gpio/
    ** Now you can change the gpio values by using below commands also.
    ** echo 1 > /sys/class/gpio/gpio21/value (turn ON the LED)
    ** echo 0 > /sys/class/gpio/gpio21/value (turn OFF the LED)
    ** cat /sys/class/gpio/gpio21/value (read the value LED)
    **
    ** the second argument prevents the direction from being changed.
    */
    for (int i = 0; i < NUM_LEDS; i++) 
    {
        gpio_export(gpio_pins[i], false);
    }

    pr_info("Device Driver Insert...Done!!!\n");
    return 0;

r_gpio:
    for (int i = 0; i < NUM_LEDS; i++) 
    {
        gpio_free(gpio_pins[i]);  // 釋放每個 GPIO
    }

r_device:
    device_destroy(dev_class,dev);
r_class:
    class_destroy(dev_class);
r_del:
    cdev_del(&etx_cdev);
r_unreg:
    unregister_chrdev_region(dev,1);

    return -1;
}

/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
    for (int i = 0; i < NUM_LEDS; i++) 
    {
        gpio_unexport(gpio_pins[i]);  // 釋放每個 GPIO
    }

    for (int i = 0; i < NUM_LEDS; i++) 
    {
        gpio_free(gpio_pins[i]);  // 釋放每個 GPIO
    }    

    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - GPIO Driver");
MODULE_VERSION("1.32");
