# C-Webserver
A multithreaded webserver coded in C

## Compile program
```bash
$ cd code
$ gcc webserver.c -o server -lpthread
```

## Run webserver
```bash
$ ./server [port] [dir=.]
$ ./server 8080
```

**Note:** dir argument is optional