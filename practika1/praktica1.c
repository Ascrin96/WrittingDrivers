#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h> /* Определения макросов */
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h> /* put_user */
#include <linux/moduleparam.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/kthread.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Loktev M.A.");
MODULE_DESCRIPTION("This is practika for A.K.Rusalin");
MODULE_SUPPORTED_DEVICE( "test" ); 

DECLARE_WAIT_QUEUE_HEAD(wq);

#define DEVICE_NAME "test"
#define CLASS_NAME "my_class"
#define SUCCESS 0
#define IOCTL_GET_SIZE_BUF 500
#define IOCTL_RESET_BUF 501
#define IOC_MAGIC 'h'


static int Major;
static int Device_Open = 0;
static int size_buf = 128;
static char msg[128];
static int count = 0;
static char* msg_Ptr = msg;
static int param = 0;
static int wait_queue_flag = 0;

module_param(param, int, 0644);

static int device_open( struct inode *, struct file * );
static int device_release( struct inode *, struct file * );
static ssize_t device_read( struct file *, char *, size_t, loff_t * );
static ssize_t device_write( struct file *, const char *, size_t, loff_t * );

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

static struct device * dev;
static struct class * myClass;

static int thread(void *data);
static struct task_struct *ts;


int init_module(void){
    Major = register_chrdev(0, DEVICE_NAME, &fops);
    if(Major < 0){
    	printk(KERN_ALERT"Registering char device failed with %d\n", Major);
    }
    
    myClass = class_create(THIS_MODULE, CLASS_NAME);
    dev = device_create(myClass, NULL, MKDEV(Major, 0),NULL, DEVICE_NAME);
    printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Param %d\n", param);
    return SUCCESS;
}

void cleanup_module(void){
	device_destroy(myClass, dev);
	class_unregister(myClass);
	class_destroy(myClass);
	unregister_chrdev(Major, DEVICE_NAME);
    printk(KERN_INFO"Cleanup_module OK\n");
   	
}

static int device_open(struct inode *inode, struct file *file){
	static int counter = 0;
	if (Device_Open)
		return -EBUSY;
	Device_Open++;
//printf(msg, "I already told you %d times Hello world!\n", counter++);
//sg_Ptr = msg;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file){
	Device_Open--; /* We're now ready for our next caller */
		/** Decrement the usage count, or else once you opened the file, you'll
			never get get rid of the module. */
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t device_read(struct file *filp,char *buffer, /* buffer to fill with data */size_t length, /* length of the buffer */loff_t * offset){
	// int bytes_read = 0; /* Number of bytes actually written to the buffer*/
	// /* If we're at the end of the message, * return 0 signifying end of file */
	// if (*msg_Ptr == 0) return 0;
	// /* Actually put the data into the buffer */
	// while (length && *msg_Ptr) {
	// * The buffer is in the user data segment, not the kernel
	// * segment so "*" assignment won't work. We have to use
	// * put_user which copies data from the kernel data segment to
	// * the user data segment. 
	// 	put_user(*(msg_Ptr++), buffer++);
	// 	length--;
	// 	bytes_read++;
	// }
	// /* Most read functions return the number of bytes put into the buffer */

	int to_read = count < length ? count : length;
	if(to_read == 0){
		wait_event_killable(wq, wait_queue_flag != 0);
	}
	wait_queue_flag = 0;
	copy_to_user(buffer, msg, to_read);
	count -= to_read;
	return to_read;
}

static ssize_t device_write(struct file *fl, const char *buffer, size_t length, loff_t * offset){
	// printk(KERN_INFO"Trying to write %.*swith length %d\n",length, buffer, length);
	//sprintf(msg, "%.*s", length, buffer);
	int to_write = length < 128 ? length : 128;
	copy_from_user(msg, buffer, to_write);
	count = count + to_write;
	wait_queue_flag = 1;
    wake_up(&wq);
	//printk(KERN_INFO"WRITE\nbuffer: %s\nmsg: %s", buffer, msg);
	return to_write;
}

// static int thread(void *data){
// 	int counter = 0;
// 	while(1){
// 		printk("Hello, I am kernel thread! Message number: %d", counter++);
// 		ssleep(10);
// 		if(kthread_should_stop()){
// 			break;
// 		}
// 	}
// 	return 0;
// }