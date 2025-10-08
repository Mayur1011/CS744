# http-load-generator

Make sure your server is running before you run the load generator.
Please check README.md in the simple-webserver folder for more information.

`generator.c` has skeleton code for your solution.

To compile the load generator, use `make` and run it using `./generator`.

To run the load generator for stressing the webserver with different number of users, use the following command:

```bash
bash ./run_generator.sh
```

You have to vary the think time in the script to generate different think time setups for load generation.

You can find the results in the results folder.

## Advanced Usage (Not required for the exam)

Set the following configuration values in `http_client.h`:

- `HOST`: The host to send requests to.
- `URL`: The URL to send requests to.
- `PORT`: The port to send requests to.
- `SANITY_CHECK`: The expected response head.
- `OUTPUT:` The expected response body.
- `FAULT_EXIT:` 0 for tolerant, 1 for strict.

Then, set the following environment variables in `run_generator.sh`:

- `USER_COUNT:` The number of users to simulate.
- `THINK_TIME:` The think time between requests.
- `TEST_DURATION:` The duration of the test in seconds.
- `CPU:` The CPU to run the load generator on.

> CPU is the CPU number, not the CPU name. You can find the CPU number by running `lscpu`.
> The use the loadgenerator on the same device as the HTTP server, use mutual exclusive CPUs. Use `taskset` to set the CPU affinity of the http server and the load generator.
