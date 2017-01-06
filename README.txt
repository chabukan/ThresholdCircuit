ビルド

cudd.3.0.0内で 

./comfigure
make
make check
./configure CC=clang CXX=clang++ --enable-silent-rules --enable-shared --enable-dddmp --enable-obj
make -j4 check
sudo make install

main内で ./compile



