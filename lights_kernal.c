/*
	Final Project Kernal space:
		* Message: (light#)(ON/OFF(!/@))
		* light# representation: 1 = Red , 2 = Yellow , 3 = Green
		* To turn on a light the message must be "(light#)@"
		* To turn off a light the message must be "(light#)!"
		* Message is recieved from User space(RTUs)
 */
 
 
 /*
 You can compile the module using the Makefile provided. Just add
 obj-m += Lab6_cdev_kmod.o

 This Kernel module prints its "MajorNumber" to the system log. The "MinorNumber"
 can be chosen to be 0.

 
 -----------------------------------------------------------------------------------
 Broadly speaking: The Major Number refers to a type of device/driver, and the
 Minor Number specifies a particular device of that type or sometimes the operation
 mode of that device type. On a terminal, try:
    ls -l /dev/
 You'll see a list of devices, with two numbers (among other pieces of info). For
 example, you'll see tty0, tty1, etc., which represent the terminals. All those have
 the same Major number, but they will have different Minor numbers: 0, 1, etc.
 -----------------------------------------------------------------------------------

 After installing the module,

 1) Check the MajorNumber using dmesg

 2) You can then create a Character device using the MajorNumber returned:
	  sudo mknod /dev/YourDevName c MajorNumber 0
    You need to create the device every time the Pi is rebooted.

 3) Change writing permissions, so that everybody can write to the Character Device:
	  sudo chmod go+w /dev/YourDevName
    Reading permissions should be enabled by default. You can check using
      ls -l /dev/YourDevName
    You should see: crw-rw-rw-

 After the steps above, you will be able to use the Character Device.
 If you uninstall your module, you won't be able to access your Character Device.
 If you install it again (without having shutdown the Pi), you don't need to
 create the device again --steps 2 and 3--, unless you manually delete it.

 Note: In this implementation, there is no buffer associated to writing to the
 Character Device. Every new string written to it will overwrite the previous one.
*/

#ifndef MODULE
#define MODULE
#endif
#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/kthread.h>	// for kthreads
#include <linux/sched.h>	// for task_struct
#include <linux/time.h>		// for using jiffies
#include <linux/fs.h>
#include <asm/uaccess.h>


#define MSG_SIZE 40
#define CDEV_NAME "RTU_Kernal"	// "YourDevName"

MODULE_LICENSE("GPL");


unsigned long *GPFSEL, *GPSET, *GPCLR;
static int major;
char lightNum,value;
static char msg[MSG_SIZE], send_msg[MSG_SIZE];


static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
	// Whatever is in msg will be placed into buffer, which will be copied into user space
		ssize_t dummy = copy_to_user(buffer, send_msg, length);	// dummy will be 0 if successful
		printk("Send this note to User space: %s\n", send_msg);
		// msg should be protected (e.g. semaphore). Not implemented here, but you can add it.
		send_msg[0] = '\0';	// "Clear" the message, in case the device is read again.
					// This way, the same message will not be read twice.
					// Also convenient for checking if there is nothing new, in user space.

	return length;
}


// Function called when the user space program writes to the Character Device.
// Some arguments not used here.
// buff: data that was written to the Character Device will be there, so it can be used
//       in Kernel space.
// In this example, the data is placed in the same global variable msg used above.
// That is not needed. You could place the data coming from user space in a different
// string, and use it as needed...
static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off)
{
	ssize_t dummy;

	if(len > MSG_SIZE)
		return -EINVAL;

	// unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);
	dummy = copy_from_user(msg, buff, len);	// Transfers the data from user space to kernel space
	if(len == MSG_SIZE)
		msg[len-1] = '\0';	// will ignore the last character received.
	else
		msg[len] = '\0';

	// You may want to remove the following printk in your final version.
	printk("Message from user space: %s\n", msg);
	
	
	lightNum = msg[0];
	value = msg[1];
	
	// Red
	if(lightNum == '1'){
		if(value == '@'){
			*GPSET = *GPSET | 0x04;
		}
		else if(value == '!'){
			*GPCLR = *GPCLR | 0x04;
		}
	}
	//Yellow
	else if(lightNum == '2'){
		if(value == '@'){
			*GPSET = *GPSET | 0x08; 
		}
		else if(value == '!'){
			*GPCLR = *GPCLR | 0x04;
		}
	}
	//Green
	else if(lightNum == '3'){
		if(value == '@'){
			*GPSET = *GPSET | 0x10; 
		}
		else if(value == '!'){
			*GPCLR = *GPCLR | 0x04;
		}
	}

	return len;		// the number of bytes that were written to the Character Device.
}

// structure needed when registering the Character Device. Members are the callback
// functions when the device is read from or written to.
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
};

int cdev_module_init(void)
{
	
	GPFSEL = (unsigned long *)ioremap(0x3F200000,4096);
	// Set light Red,Yellow,Green as output
	*GPFSEL = *GPFSEL | 0x1240;
	// Set register
	GPSET = GPFSEL + (0x1c/4);
	// Clear register
	GPCLR = GPFSEL + (0x28/4);

	// register the Characted Device and obtain the major (assigned by the system)
	major = register_chrdev(0, CDEV_NAME, &fops);
	if (major < 0) {
     		printk("Registering the character device failed with %d\n", major);
	     	return major;
	}
	printk("RTUs_cdev_kmod, assigned major: %d\n", major);
	printk("Create Char Device (node) with: sudo mknod /dev/%s c %d 0\n", CDEV_NAME, major);
 	return 0;
}

void cdev_module_exit(void)
{
	iounmap(GPFSEL);
	// Once unregistered, the Character Device won't be able to be accessed,
	// even if the file /dev/YourDevName still exists. Give that a try...
	unregister_chrdev(major, CDEV_NAME);
	printk("Char Device /dev/%s unregistered.\n", CDEV_NAME);
}

module_init(cdev_module_init);
module_exit(cdev_module_exit);

