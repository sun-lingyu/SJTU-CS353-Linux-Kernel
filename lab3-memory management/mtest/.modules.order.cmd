cmd_/root/mtest/modules.order := {   echo /root/mtest/mtest.ko; :; } | awk '!x[$$0]++' - > /root/mtest/modules.order
