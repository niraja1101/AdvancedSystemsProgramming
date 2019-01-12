
obj-m := assignment6.o

KERNEL_DIR = /usr/src/linux-headers-$(shell uname -r)

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(shell pwd)
	
#app: 
	gcc -o child child.c
	gcc -o deadlock1 deadlock1.c
	gcc -o deadlock2 deadlock2.c
	gcc -o deadlock3 deadlock3.c -pthread
	gcc -o deadlock4 deadlock4.c -pthread

clean:
	rm -rf deadlock1 deadlock2 deadlock3 deadlock4 child *.o *.ko *.mod.* *.symvers *.order *~ 
