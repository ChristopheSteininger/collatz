g++ -c -Wall -I /home/christophe/tools/AMD-APP-SDK-v2.8-RC-lnx32/include vectoradd.cpp -o vectoradd.o
g++ vectoradd.o -o vectoradd -L /home/christophe/tools/AMD-APP-SDK-v2.8-RC-lnx32/lib/x86 -l OpenCL

echo "Done"
