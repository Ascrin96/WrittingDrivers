#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h> /* Определения макросов */
#include <linux/fs.h>
#include <asm/uaccess.h> /* put_user */


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Loktev M.A.");
MODULE_DESCRIPTION("The task 2 on course Writting Drivers");
MODULE_SUPPORTED_DEVICE( "test" ); 

#define DEVICE_NAME "test"
#define SUCCESS 0

static int Major;
static int Device_Open = 0;
static char msg[128];
static char* msg_Ptr = msg;

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

int init_module(void){
    Major = register_chrdev(0, DEVICE_NAME, &fops);
    if(Major < 0){
    	printk(KERN_ALERT"Registering char device failed with %d\n", Major);
    }
    printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
    return SUCCESS;
}

void cleanup_module(void){
	unregister_chrdev(Major, DEVICE_NAME);
    printk(KERN_INFO"Cleanup_module OK\n");
}

static int device_open(struct inode *inode, struct file *file){
	static int counter = 0;
	if (Device_Open)
		return -EBUSY;
	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
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
	int bytes_read = 0; /* Number of bytes actually written to the buffer*/
	/* If we're at the end of the message, * return 0 signifying end of file */
	if (*msg_Ptr == 0) return 0;
	/* Actually put the data into the buffer */
	while (length && *msg_Ptr) {
	/** The buffer is in the user data segment, not the kernel
	* segment so "*" assignment won't work. We have to use
	* put_user which copies data from the kernel data segment to
	* the user data segment. */
		put_user(*(msg_Ptr++), buffer++);
		length--;
		bytes_read++;
	}
	/* Most read functions return the number of bytes put into the buffer */
	return bytes_read;
}

static ssize_t device_write( struct file *filp, const char *buffer, size_t lenght, loff_t * offset ){
	int bytes_write = 0;
	if(lenght > 128) lenght = 128;
	sprintf(msg, buffer);
	msg_Ptr = msg;
 	return bytes_write;
}


