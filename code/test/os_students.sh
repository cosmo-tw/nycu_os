make distclean
make 
timeout 1 ../build.linux/nachos  -ep consoleIO_test1 60 -ep consoleIO_test2 90 
echo "done"