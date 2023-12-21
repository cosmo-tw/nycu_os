make distclean
make 
timeout 1 ../build.linux/nachos -ep consoleIO_test2 50 -ep consoleIO_test1 70 -ep consoleIO_test3 80
echo "done"