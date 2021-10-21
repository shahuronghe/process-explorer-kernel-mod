/*
 ========================================================================================
 Driver Name : PROCESS_EXPLORER_DRIVER
 Author      : SHAHU RONGHE & SURYANSH KAUSHIK
 YEAR		 : FEBRUARY 2018
 License	 : GPL
 Description : This driver program display list of processes running in the system at kernel
				and user level.
 ========================================================================================
 */

#include"chardriver_lowlevel.h"

//#include"chardriver_lowlevel_ioctl.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/sys.h>
#include<linux/list.h>
#include<linux/init_task.h>
#include<linux/slab.h>
//#include <asm/system.h>

#define SIZE 1024
#define NMINORS 1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("C-DAC");

/* Define a device specific data */
typedef struct privatedata {
	int nMinor;
	struct process_data *ptr_process_data;
	struct cdev mycdev;

} chardriver_lowlevel_private;

struct process_data
{
    unsigned long long prev_cal;
    unsigned long long cur_cal;
    unsigned long resident_size;
    char *process_name;
    int process_id;
    int parent_process_id;
    struct process_data *next;
};

chardriver_lowlevel_private devices[NMINORS];

/* Declare the required variables */
int major;
int minor = 1;
dev_t deviceno;
struct class *chardriver_lowlevel_class;
struct device *chardriver_lowlevel_device;

/* Char driver open function : Called when open() is called on the device */
int chardriver_lowlevel_open(struct inode *inod, struct file *filp)

{
	/* Assign the address of device specific structure into filp->private_data
	 * such that it can be accessed across all the functions, each pertaining to
	 * the specific device
	 */
	chardriver_lowlevel_private *dev = container_of(inod->i_cdev,
			chardriver_lowlevel_private, mycdev);
	filp->private_data = dev;

	printk("In char driver open() function device node : %d\n", dev->nMinor);

	return 0;
}

/* Char driver release function : Called when close() is called on the device */
int chardriver_lowlevel_release(struct inode *inod, struct file *filp)
{
	/* Retrieve the device specific structure */
	chardriver_lowlevel_private *dev = filp->private_data;

	printk("In char driver release() function device node : %d\n", dev->nMinor);
	return 0;
}

/* Char driver read function : Called when read() is called on the device
 * It is used to copy data to the user space.
 * The function must return the number of bytes actually transferred
 */
ssize_t chardriver_lowlevel_read(struct file *filp,char __user *Ubuff, size_t count, loff_t *offp)
{
	/* Retrieve the device specific structure */
   unsigned long start_code,end_code,start_data,end_data,code,data;
	/* Retrieve the device specific structure */
	chardriver_lowlevel_private *dev = filp->private_data;
	int res;
    int process_count=0;

    

    struct task_struct *curr_pr = current;
    struct task_struct *head=curr_pr;
    struct process_data *process_data_node;
    //struct process_data arr;

    curr_pr=next_task(curr_pr);   
    while(curr_pr!=head) {
        if(curr_pr->mm!=NULL) { 
		    process_data_node=kmalloc(sizeof(struct process_data), GFP_KERNEL);
            if(process_count==0) {
                start_code=curr_pr->mm->start_code;
                end_code=curr_pr->mm->end_code;
                start_data=curr_pr->mm->start_data;
                end_data=curr_pr->mm->end_data;
                code=(end_code-start_code)+1;
                data=(end_data-start_data)+1;
                process_data_node->resident_size=code+data;
            

                process_data_node->prev_cal = (curr_pr->prev_cputime.utime) + (curr_pr->prev_cputime.stime);
                process_data_node->cur_cal = (curr_pr->utime) + (curr_pr->stime);
           
                process_data_node->process_name=curr_pr->comm;
                process_data_node->process_id=curr_pr->pid;
                process_data_node->parent_process_id=curr_pr->parent->pid;
                process_data_node->next=NULL;
                dev->ptr_process_data=process_data_node;
				process_count++;
            } else {
                struct process_data *temp;
                temp=dev->ptr_process_data;
                while(temp->next!=NULL) {
                    temp=temp->next;
                }
                
                start_code=curr_pr->mm->start_code;
                end_code=curr_pr->mm->end_code;
                start_data=curr_pr->mm->start_data;
                end_data=curr_pr->mm->end_data;
                code=(end_code-start_code)+1;
                data=(end_data-start_data)+1;
                process_data_node->resident_size=code+data;
            
                process_data_node->prev_cal = (curr_pr->prev_cputime.utime) + (curr_pr->prev_cputime.stime);
                process_data_node->cur_cal = (curr_pr->utime) + (curr_pr->stime);
           
                process_data_node->process_name=curr_pr->comm;
                process_data_node->process_id=curr_pr->pid;
                process_data_node->parent_process_id=curr_pr->parent->pid;
                process_data_node->next=NULL;
                temp->next=process_data_node;
				process_count++;                
            }
        }
        curr_pr=next_task(curr_pr);

    }   
	
	struct process_data arr[process_count];
	struct process_data *temp;        
	temp = dev->ptr_process_data;
	int i = 0;
	while(temp!=NULL) {
		arr[i]=*temp;
		temp=temp->next;
		i++;
	}
	//printk(KERN_NOTICE "above copy to user %s resident %d \n",arr.process_name,arr.resident_size);   

	/* copy the data from kernel buffer to User-space buffer */
	res = copy_to_user((struct process_data *)Ubuff , (struct process_data *)arr, sizeof(struct process_data)*process_count);
	if(res == 0) {
		printk("\ndata from kernel buffer to user buffer copied successfully with bytes : %d\n",process_count);
		return process_count;
	} else {
		printk("copy from kernel to user failed\n");
		return -EFAULT;
	}
	return 0;
}

/* Char driver read function : Called when read() is called on the device
 * It is used to copy data to the user space.
 * The function must return the number of bytes actually transferred
 */
ssize_t chardriver_lowlevel_write(struct file *filp, const char __user *Ubuff,size_t count, loff_t *offp){
	/*Copy data from user space buffer to driver buffer 
	//memset(dev->Kbuff,0,sizeof(dev->Kbuff));
	res = copy_from_user((char *)dev->Kbuff , (char *) Ubuff,count);
	if(res == 0) {
		printk("data from the user space : %s no of bytes : %d\n",dev->Kbuff, count);
		return count;
	} else {
		printk("copy from user space to kernel failed\n");
		return -EFAULT;
	}
	*/
	return 0;
}

/* Define the file operations structure of the char driver */
struct file_operations chardriver_lowlevel_fops= {
	.owner 			= THIS_MODULE,
	.open			= chardriver_lowlevel_open,
	.release		= chardriver_lowlevel_release,
	.read			= chardriver_lowlevel_read,
	.write			= chardriver_lowlevel_write,
	
};

/* Initialisation function */
static int __init chardriver_lowlevel_init(void) {
	int i;
	int res;

	/* Get the device number dynamically */
	res = alloc_chrdev_region(&deviceno , minor, NMINORS , DRIVER_NAME);
	if(res <0) {
		PERR("register device no failed\n");
		return -1;
	}
	major = MAJOR(deviceno);

	/* create a class file with the name DRIVER_NAME such that it
	 *  will appear in /sys/class/<DRIVER_NAME
	 */
	chardriver_lowlevel_class = class_create(THIS_MODULE , DRIVER_NAME);
	if(chardriver_lowlevel_class == NULL) {
		PERR("class creation failed\n");
		return -1;
	}

	/* Create nMinors Device nodes , such users can access through nodes Ex :/dev/sample_cdev0 */
	for(i = 0; i < NMINORS; i++) {
		deviceno = MKDEV(major, minor + i);
		/* Attach file_operations to cdev and add the device to the linux kernel */
		cdev_init(&devices[i].mycdev , &chardriver_lowlevel_fops);
		cdev_add(&devices[i].mycdev, deviceno,1);

		/* Create the Device node in /dev/ directory */
		chardriver_lowlevel_device = device_create(chardriver_lowlevel_class ,
				NULL , deviceno , NULL ,"sample_cdev%d",i);
		if(chardriver_lowlevel_device == NULL) {
			class_destroy(chardriver_lowlevel_class);
			PERR("device creation failed\n");
			return -1;
		}
		devices[i].nMinor = minor + i;
	}
	return 0;
}

/* Cleanup function */
static void __exit chardriver_lowlevel_exit(void) {
	/* Remove cdev and device nodes with linux kernel */
	for(int i = 0; i < NMINORS; i++) {
		deviceno = MKDEV(major, minor+i);
		cdev_del(&devices[i].mycdev);
		device_destroy(chardriver_lowlevel_class , deviceno);
	}

	/* Destroy the class we have created */
	class_destroy(chardriver_lowlevel_class);

	/* Unregister the device number from linux kernel */
	deviceno=MKDEV(major, minor);
	unregister_chrdev_region(deviceno, NMINORS);
}

module_init(chardriver_lowlevel_init)
module_exit(chardriver_lowlevel_exit)
