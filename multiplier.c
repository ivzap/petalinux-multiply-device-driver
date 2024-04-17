/*  my_chardev.c - Simple character device module
 *   *  
 *	*  Demonstrates module creation of character device for user
 * 	*  interaction.
 *  	*
 *   	*  (Adapted from various example modules including those found in the
 *    	*  Linux Kernel Programming Guide, Linux Device Drivers book and
 *     	*  FSM's device driver tutorial)
 *      	*/

/* Moved all prototypes and includes into the headerfile */
#include "multiplier.h"

static struct file_operations fops = {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release
};

void *virtaddr;

/*
 * This function is called when the module is loaded and registers a
 * device for the driver to use.
 */
int my_init(void)
{

  msg_buf_Ptr = (char *)kmalloc(BUF_LEN*sizeof(char), GFP_KERNEL);
 
  if (msg_buf_Ptr == NULL) {
	/* Failed to get memory, exit gracefully */
	printk(KERN_ALERT "Unable to allocate needed memory\n");

	return 10;  
  }

  // clear the stream for future use
  memset(msg_buf_Ptr, 0, BUF_LEN);

  virtaddr = ioremap(PHY_ADDR, MEMSIZE);
  Major = register_chrdev(0, DEVICE_NAME, &fops);
   
  /* Negative values indicate a problem */
  if (Major < 0) {   	 
	printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	return Major;
  }

  printk(KERN_INFO "Registered a device with dynamic Major number of %d\n", Major);
 
  printk(KERN_INFO "Create a device file for this device with this command:\n'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);

  return 0;   	 /* success */
}

/*
 * This function is called when the module is unloaded,
 * it releases the device file.
 */
void my_cleanup(void)
{
  /*
  * Unregister the device
  */
  unregister_chrdev(Major, DEVICE_NAME);
  printk(KERN_ALERT "Unmapping virtual address space...\n");
  iounmap((void *)virtaddr);
  /* free our memory (note the ordering)*/
  kfree(msg_buf_Ptr);

}


/*
 * Called when a process tries to open the device file, like "cat
 * /dev/my_chardev".  Link to this function placed in file operations
 * structure for our device file.
 */ 
static int device_open(struct inode *inode, struct file *file)
{
  printk(KERN_ALERT "Opening multiplier device driver...\n");
  return 0;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
  printk(KERN_ALERT "Closing multiplier device driver...\n");   
  return 0;
}

/*
 * Called when a process, which already opened the dev file, attempts
 * to read from it.
 */
static ssize_t device_read(
        struct file *filp, /* see include/linux/fs.h*/
        char *buff,  	/* buffer to fill with data */
        size_t length, 	/* length of thebuffer */
        loff_t * offset)
{
  // Do we have enough space to transfer three integers?
  int bytes_read = 0;

  int reg0 = ioread32(virtaddr + 0);
  int reg1 = ioread32(virtaddr + 4);
  int reg2 = ioread32(virtaddr + 8);

  char *reg0_bytes = (char*)&reg0;
  char *reg1_bytes = (char*)&reg1;
  char *reg2_bytes = (char*)&reg2;
  // copy product & args into byte stream
  // | arg1 | arg2 | prod |
  // 0      4      8      12
  memcpy(msg_buf_Ptr+0*sizeof(int), reg0_bytes, sizeof(int));
  memcpy(msg_buf_Ptr+1*sizeof(int), reg1_bytes, sizeof(int));
  memcpy(msg_buf_Ptr+2*sizeof(int), reg2_bytes, sizeof(int));
  msg_buf_Ptr[BUF_LEN-1] = '\0';
  int i;
  for(i = 0; i < min(BUF_LEN, length); i++){
    put_user(*(msg_buf_Ptr+i), buff++); /* one char at a time... */
    bytes_read++;
  }
  // clear the stream for future use
  memset(msg_buf_Ptr, 0, BUF_LEN);
  return bytes_read;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello
 * Next time we'll make this one do something interesting.
 * 	
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t length, loff_t * off)
{
  int i;
 
  /* printk(KERN_INFO "device_write(%p,%s,%d)", file, buffer, (int)length); */
 
  /* get_user pulls message from userspace into kernel space */
  for (i = 0; i < length && i < 2*sizeof(int); i++)
    // retrieve data from user buff and place into msg_buf_Ptr
	  get_user(msg_buf_Ptr[i], buff + i);
 
  /* left one char early from buffer to leave space for null char*/
  msg_buf_Ptr[i] = '\0';
  int inputs[2];
  memcpy(inputs, msg_buf_Ptr, 2*sizeof(int));
  printk("device_write(): input[0]: %d\n", inputs[0]);
  printk("device_write(): input[1]: %d\n", inputs[1]);
  // clear the stream for future use
  memset(msg_buf_Ptr, 0, BUF_LEN);

  // write directly to hardware IP
  iowrite32(inputs[0], virtaddr + 0);
  iowrite32(inputs[1], virtaddr + 4);

  return i;
}

/* These define info that can be displayed by modinfo */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ivan Zaplatar");
MODULE_DESCRIPTION("Module which creates a character device and allows user interaction with it");

/* Here we define which functions we want to use for initialization
 *	and cleanup */
module_init(my_init);
module_exit(my_cleanup);
