# Simple Webserver

A simple C++ based WebServer with Pool Threads and HTTP Processing.

## Usage

For the exam you don't need to change anything in the code.
Just run the following commands in the terminal to run the server pinned to a single CPU:
```console
make
taskset -c 0 ./server
```

In the above example, the server is isolated to run on the first CPU (0) only. This makes sure that you can use lab machines to run the server and the load generator on the same machine and still get accurate results.

Open `localhost:8080` in your browser to see the server in action. Alternatively, you can use `curl localhost:8080` to send requests to the server.
