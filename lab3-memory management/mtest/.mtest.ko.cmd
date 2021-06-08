cmd_/root/mtest/mtest.ko := ld -r  -EL  -maarch64elf  --build-id=sha1  -T scripts/module.lds -o /root/mtest/mtest.ko /root/mtest/mtest.o /root/mtest/mtest.mod.o;  true
