make distclean
make 
timeout 1 ../build.linux/nachos  -ep consoleIO_test1 120 -ep consoleIO_test2 130
echo "done"