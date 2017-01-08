#!/bin/bash
source ./run.sh

(cd ../bdd/
#make clean
run make
cd ../network/
make clean
run make
cd ../util/
#make clean
run make
cd ../main/
make clean
run make
cd ..
#run gtags
#run gtags --gtagslabel=exuberant-ctags
cd ./main/
#./ger-program benchmark/test_cspf_red.log
#./ger-program benchmark/usb_phy_maxfanin.log #GerTemp.blif
#./ger-program benchmark/simple_spi_maxfanin.log
#./ger-program benchmark/alu4_maxfanin.log 
#./ger-program benchmark/apex6_maxfanin.log 
#./ger-program benchmark/k2_maxfanin.log 
./ger-program benchmark/dalu_maxfanin.log 
#./ger-program benchmark/C1908_maxfanin.log #> resul2.txt
#./ger-program benchmark/i2c_maxfanin.log
#./ger-program benchmark/pci_spoci_ctrl_maxfanin.log
#./ger-program benchmark/rot_maxfanin.log #> result.txt
)
