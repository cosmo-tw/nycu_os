make distclean
make 
timeout 2 ../build.linux/nachos -e consoleIO_test1 -e consoleIO_test2
echo "done"