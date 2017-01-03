### A simple thread pool

The threadpool starts N threads and each of them picks up work from a queue of work tasks.

The interface is the following:

`pool_init(N)` - Returns a thread pool with capacity of N threads.
`pool_get_max_threads` - Returns the thread capacity of the pool.
`pool_add_task` - Adds a task to the thread pool.
`pool_wait` - Waits until all tasks are completed.
`pool_destroy` - Destroys the thread pool.

See this [test](test_threadpool.c) for execution example.
