make distclean
make 
timeout 1 ../build.linux/nachos  -ep consoleIO_test1 10 -ep consoleIO_test2 20
echo "done"