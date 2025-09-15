# Circular Wait (Dining Philosophers)
At the root of the dining philosopher problem is a "circular wait" deadlock issue. This folder explores solutions to this same issue.

I've contextualized the problem as a consumer/producer problem because that's typically what I see most in the wild in my lived experience. You have some large system with chunks of data being handled by separate distinct pieces. In my experience the vast majority of problems that involve mutexes/semaphores that I see can be re-contextualized as worker threads that communicate via producing/consuming from some shared resource.

The scenario in this folder is one with 3 workers, which each consume from a shared resource, do some work, and then produce to another shared resource. The 1st worker consumes the intial data, does work to it, then produces data to the 2nd worker. The 2nd worker consumes data from the 1st worker, then produces data to the 3rd worker. The 3rd worker consumes data from the 2nd worker, then in a typical scenario, the process would end here. The data is written to a file, displayed to a screen, etc. To keep this scenario a bit more interesting, the 3rd worker produces data back to the 1st worker.

Scenario:
* 3 workers (threads)
* 3 queues (shared resources)
* Each worker reads data from a queue, does some work to it, then writes data to the next queue.
  * Worker1: consume queue1, do work, produce to queue2
  * Worker2: consume queue2, do work, produce to queue3
  * Worker3: consume queue3, do work, produce to queue1

## deadlock.c
An example of a system with 3 workers and 3 shared resources, each worker needing 2 specific shared resources and a naive concurrency implementation that causes a deadlock using mutexes with blocking aquires.

* Scenario:
  * Worker1: aquire lock to access queue1, aquire lock to access queue2, do work
  * Worker2: aquire lock to access queue2, aquire lock to access queue3, do work
  * Worker3: aquire lock to access queue3, aquire lock to access queue1, do work
* Result:
  * Worker1 has lock to queue1, but is waiting on lock to queue2
  * Worker2 has lock to queue2, but is waiting on lock to queue3
  * Worker3 has lock to queue3, but is waiting on lock to queue1
  * All workers are stuck waiting on a resource, the system is in a deadlock because of a "circular wait".
    * A solution to this is to use a "ordered lock" schema (see `ordered_lock_schema.c`).


# livelock.c
An example of a system with 3 workers and 3 shared resources, each worker needing 2 specific shared resources and a naive concurrency implementation that causes livelock using mutexes with non-blocking aquires.

* Scenario:
  * Worker1: non-blocking aquire lock to access queue1, non-blocking aquire lock to access queue2, do work
  * Worker2: non-blocking aquire lock to access queue2, non-blocking aquire lock to access queue3, do work
  * Worker3: non-blocking aquire lock to access queue3, non-blocking aquire lock to access queue1, do work
* Result:
  * Worker1 aquires queue1 lock
  * Worker2 aquires queue2 lock
  * Worker3 aquires queue3 lock
  * Worker1 tries to aquire queue2 lock but fails and releases queue1 lock
  * Worker2 tries to aquire queue3 lock but fails and releases queue2 lock
  * Worker3 aquires queue1 lock, completes it's work and releases both locks
  * repeats
  * some workers are never able to obtain both locks they need to do work, they are "starved". this leads eventually to all workers being able to do any work and we end up in a "livelock" state because certain workers are starved of the shared resource.
    * A trivial/hacky solution to this is to add random sleep amounts between waiting for shared resource mutexes. It will solve the livelock issue, but is not ideal.


# ordered_lock_schema.c
An example that overcomes the deadlock caused by the naive solution. The schema implemented here is to always try to aquire the lowest id lock first. This avoids the circular wait deadlock problem.

* Scenario:
  * Worker1: aquire lock to access queue1, aquire lock to access queue2, do work
  * Worker2: aquire lock to access queue2, aquire lock to access queue3, do work
  * Worker3: aquire lock to access queue1, aquire lock to access queue3, do work
* Result:
  * Worker1 aquires lock to access queue1
  * Worker2 aquires lock to access queue2
  * Worker3 attempts to aquire lock to access queue1, but can't because worker 1 has it
  * Worker 2 aquires lock to access queue3, finishes it's work and releases keys
  * Worker 1 aquires lock to access queue2, finishes it's work and releases keys
  * Worker 3 aquires lock to access queue1 and queue3, finishes it's work and releases keys.
  * repeats


# lock_free_queue.c
In a non-trivial system, it is hard to figure out which exact workers and shared resources have a "circular wait" problem. Most problems that involve locks in the spaces I've worked can be re-formatted to use lock-free solutions like atomic CAS queues or "mailbox" style messaging tools for example. "Communicate to share, don't communicate by sharing."

An example using lock-free atomic queues to avoid the deadlock/livelock problems without complicated lock ordering schemas that are easy to make mistakes with. CAS - compare and swap with atomics. potential issues: aba problem.
