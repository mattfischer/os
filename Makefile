qemu:
	@echo "Starting QEMU..."
	@qemu-system-arm -M integratorcp -kernel out/kernel/kernel -nographic

qemu-gdb:
	@echo "Starting QEMU..."
	@qemu-system-arm -M integratorcp -kernel out/kernel/kernel -s -S -nographic

gdb:
	@echo "target remote :1234" > /tmp/gdbinit
	@arm-eabi-gdb out/kernel/kernel -x /tmp/gdbinit