# Compile the tests
```
make all
```

## To run monolithic test (test functionality of system calls)
```
make run_monolithic
```

## To run the a race condition test to see if the mutex is working properly
```
make run_init
make run_race
make run_shut
```

## You can check dmesg for robust error handling, to see more details about each error
```
sudo dmesg
```