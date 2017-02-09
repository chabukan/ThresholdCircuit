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

INPUTDIR="./benchmark/long/not_used"
INPUT=(`ls ${INPUTDIR}`)
OUTPUTDIR=./result
PROGRAM="./ger-program"

TIME=10000
SIGNAL="KILL"

for file in ${INPUT[@]}
do
    echo "INPUTFILE : " ${file}
    OUTPUT=${OUTPUTDIR}/${file}
    echo "timeout ${TIME} ${PROGRAM} ${INPUTDIR}/${file} > ${OUTPUT}"
    timeout -s ${SIGNAL} ${TIME} ${PROGRAM} ${INPUTDIR}/${file} > ${OUTPUT}
    if [ $? -eq 124 ];
    then
    echo "${file} running Timeover. MAXTIME:${TIME}s"
    fi
done


#./ger-program benchmark/test_cspf_red.log
#./ger-program benchmark/usb_phy_maxfanin.log #GerTemp.blif
#./ger-program benchmark/simple_spi_maxfanin.log
#./ger-program benchmark/alu4_maxfanin.log #> memo.txt
#./ger-program benchmark/apex6_maxfanin.log 
#./ger-program benchmark/k2_maxfanin.log 
#./ger-program benchmark/dalu_maxfanin.log 
#./ger-program benchmark/C1908_maxfanin.log #> resul2.txt
#./ger-program benchmark/i2c_maxfanin.log
#./ger-program benchmark/pci_spoci_ctrl_maxfanin.log
#./ger-program benchmark/rot_maxfanin.log #> result.txt

#./ger-program benchmark/des_maxfanin.log
#./ger-program benchmark/pair_maxfanin.log
#./ger-program benchmark/systemcdes_maxfanin.log
)
