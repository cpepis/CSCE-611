# Replace "/mnt/floppy" with the whatever directory is appropriate.
make clean
make
sudo mount -o loop dev_kernel_grub.img /mnt/floppy
sudo cp kernel.elf /mnt/floppy
sleep 1s
sudo umount /mnt/floppy
sleep 1s
gnome-terminal -- bochs -f bochsrc.bxrc