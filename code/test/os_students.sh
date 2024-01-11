make distclean
make 
timeout 1 ../build.linux/nachos  -ep hw4_normal_test1 70 -ep hw4_normal_test2 50 -ep hw4_normal_test3 80
echo "done"