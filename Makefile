qemu:
	@echo "Starting QEMU..."
	@qemu-system-arm -kernel out/kernel/kernel -s -S

gdb:
	@echo "target remote :1234" > /tmp/gdbinit
	@arm-none-eabi-gdb out/kernel/kernel -x /tmp/gdbinit