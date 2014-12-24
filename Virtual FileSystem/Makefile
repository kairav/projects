all: ramdisk

ramdisk: ramdisk.c
	gcc -Wall ramdisk.c -o ramdisk `pkg-config fuse --cflags --libs` 

clean:
	rm -rf ramdisk.o ramdisk
