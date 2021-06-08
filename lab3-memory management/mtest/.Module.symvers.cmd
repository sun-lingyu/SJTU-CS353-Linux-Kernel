cmd_/root/mtest/Module.symvers := sed 's/\.ko$$/\.o/' /root/mtest/modules.order | scripts/mod/modpost  -a   -o /root/mtest/Module.symvers -e -i Module.symvers   -T -
