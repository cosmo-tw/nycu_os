make distclean
make 
timeout 1 ../build.linux/nachos  -ep consoleIO_test1 30 -ep consoleIO_test2 20 -ep consoleIO_test3 10 
echo "done"