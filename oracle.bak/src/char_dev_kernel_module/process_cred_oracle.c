/**
 * Character Device driver (Kernel Module)
 * 
 * @author Harshit Vadodaria (harshitv95@gmail.com)
 * @version 1.0
 * */

#include <linux/kernel.h>
// #include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched/signal.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

int init_module(void);
void cleanup_module(void);
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);



#define DEVICENAME "processlist"

static int device_major;
static dev_t dev_num;
struct class *dev_class;
struct device *dev;

static int device_open = 0;


static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release
};

static char *devnode(struct device *dev, umode_t *mode) {
    if (!mode) return NULL;
    *mode = 0666;
    return NULL;
}

int init_module() {
    if ((device_major = register_chrdev(0, DEVICENAME, &fops)) <= 0) {
        printk(KERN_ALERT "Reigstering device %s failed with %d\n", DEVICENAME, device_major);
        return device_major;
    }
    else {
        printk(KERN_INFO "Device %s registered with major %d\n", DEVICENAME, device_major);
        
        // Creating Class (sys/class/{DEVICENAME})
        dev_num = MKDEV(device_major, 0);
        dev_class = class_create(THIS_MODULE, DEVICENAME);
        if (IS_ERR(dev_class)) {
            printk(KERN_WARNING "Failed to create class for device %s with major %d, unregistering", DEVICENAME, device_major);
            unregister_chrdev_region(dev_num, 1);
            return -1;
        }
        dev_class->devnode = devnode;

        // Class created successfully, now creating device node (/dev/{DEVICENAME})
        // that lets user space interact with hardware through kernel
        dev = device_create(dev_class, NULL, dev_num, NULL, DEVICENAME);
        if (IS_ERR(dev)) {
            printk(KERN_WARNING "Failed to create node for device %s with major %d, unregistering", DEVICENAME, device_major);
            // revert
            class_destroy(dev_class);
            unregister_chrdev_region(dev_num, 1);
            return -1;
        }

        return 0;
    }
}

void cleanup_module() {
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    unregister_chrdev(device_major, DEVICENAME);
    printk(KERN_INFO "Device %s with major %d un-registered\n", DEVICENAME, device_major);
}

static int dev_open(struct inode *inode, struct file *file_ptr) {
    if (device_open)
        return -EBUSY;
    device_open = 1;
    printk(KERN_INFO "Device [%s] opened\n", DEVICENAME);
    return 0;
}

#define LEN 64
char pid_buf[LEN];

static ssize_t dev_write(struct file * file_ptr, const char * buf, size_t len, loff_t * off) {
    long pid;
    struct task_struct* task;

    if (len < LEN) {
    copy_from_user(pid_buf, buf, len);
    printk(KERN_ALERT "written: %s\n", pid_buf);
    if (kstrtol(pid_buf, 10, &pid) == 0) {
        printk(KERN_ALERT "pid written:%ld\n", pid);
        for_each_process(task) {
            if (task->pid == pid) {
               /* 
                printk(KERN_ALERT "Value of task->seccomp.mode = %d, task->seccomp.filter = %p\n", task->seccomp.mode, task->seccomp.filter);
                printk(KERN_ALERT "Threadinfo flags has seccomp: %d\n", task->thread_info.flags & _TIF_SECCOMP);
                printk(KERN_ALERT "I'm setting it to zero manually, because I'm king!\n");
                task->thread_info.flags &= 0xFFFFFFFFFFFFFEFF;
                //task->thread_info.flags = 0x0;
                task->seccomp.mode = 0;
                task->seccomp.filter = 0;
                printk(KERN_ALERT "Rechecking threadinfo flags has seccomp: %d\n", task->thread_info.flags & _TIF_SECCOMP);
                printk(KERN_ALERT "Rechecking value of task->seccomp.filter = %p\n", task->seccomp.filter);
                */

                printk(KERN_ALERT "Address of capability base: %p\n", &(task->cred->cap_inheritable.cap[0]));

                /*
                printk(KERN_ALERT "Address of task->cred->cap_permitted.cap[0]: %p, value: %lx\n", &(task->cred->cap_permitted.cap[0]), task->cred->cap_permitted.cap[0]);
                printk(KERN_ALERT "Address of task->cred->cap_permitted.cap[1]: %p, value: %lx\n", &(task->cred->cap_permitted.cap[1]), task->cred->cap_permitted.cap[1]);

                printk(KERN_ALERT "Address of task->cred->cap_effective.cap[0]: %p, value: %lx\n", &(task->cred->cap_effective.cap[0]), task->cred->cap_effective.cap[0]);
                printk(KERN_ALERT "Address of task->cred->cap_effective.cap[1]: %p, value: %lx\n", &(task->cred->cap_effective.cap[1]), task->cred->cap_effective.cap[1]);

                printk(KERN_ALERT "Address of task->cred->cap_inheritable.cap[0]: %p, value: %lx\n", &(task->cred->cap_inheritable.cap[0]), task->cred->cap_inheritable.cap[0]);
                printk(KERN_ALERT "Address of task->cred->cap_inheritable.cap[1]: %p, value: %lx\n", &(task->cred->cap_inheritable.cap[1]), task->cred->cap_inheritable.cap[1]);

                printk(KERN_ALERT "Address of task->cred->cap_bset.cap[0]: %p, value: %lx\n", &(task->cred->cap_bset.cap[0]), task->cred->cap_bset.cap[0]);
                printk(KERN_ALERT "Address of task->cred->cap_bset.cap[1]: %p, value: %lx\n", &(task->cred->cap_bset.cap[1]), task->cred->cap_bset.cap[1]);

                printk(KERN_ALERT "Address of task->cred->cap_ambient.cap[0]: %p, value: %lx\n", &(task->cred->cap_ambient.cap[0]), task->cred->cap_ambient.cap[0]);
                printk(KERN_ALERT "Address of task->cred->cap_ambient.cap[1]: %p, value: %lx\n", &(task->cred->cap_ambient.cap[1]), task->cred->cap_ambient.cap[1]);

                printk(KERN_ALERT "Address of task->cred->uid: %p, value: %lx\n", &(task->cred->uid), task->cred->uid);
                */
                if (has_capability(task, CAP_SYS_ADMIN)) {
                    printk(KERN_ALERT "Has CAP_SYS_ADMIN");
                }
                // HACK!
                /*
                cap_raise(task->cred->cap_effective, 21);
                cap_raise(task->cred->cap_permitted, 21);
                */
            }
        }
    }
    }
    return 0;
}

static ssize_t dev_read(struct file * file_ptr, char * buf, size_t len, loff_t * off) {
    long pid;
    struct task_struct* task;
    char message[512];
    int error_count = 0;
    int size_of_message = 0;

    if (kstrtol(pid_buf, 10, &pid) == 0) {
        printk(KERN_ALERT "pid written:%ld\n", pid);
        for_each_process(task) {
            if (task->pid == pid) {
                sprintf(message,"Address of capability base: %p\n", &(task->cred->cap_inheritable.cap[0]));
                size_of_message = strlen(message);
                error_count = copy_to_user(buf, message, strlen(message));
                if (error_count==0){            // if true then have success
                    printk(KERN_INFO "dev: Sent %d characters to the user\n", size_of_message);
                    return 0;  // clear the position to the start and return 0
                }
                else {
                    printk(KERN_INFO "EBBChar: Failed to send %d characters to the user\n", error_count);
                    return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
                }
            }
        }
    }
    return 0;
}

static int dev_release(struct inode * node, struct file * file_ptr) {
    device_open = 0;
    printk(KERN_INFO "Device [%s] closed\n", DEVICENAME);
    return 0;
}

