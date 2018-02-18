#include <linux/module.h> 
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Loktev M.A.");
MODULE_DESCRIPTION("The task 1 on course Writting Drivers/");

int init_module(void){
	printk(KERN_INFO"Hello world\n");
	return 0;
}

void cleanup_module(void){
	printk(KERN_INFO"Goodbye world\n");
}