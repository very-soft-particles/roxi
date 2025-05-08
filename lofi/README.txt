
lofi 1.0 is experimental and contains support for a fiber based job system with lock free data structures such as a lock free pool which utilizes atomic CAS and Jason Gregory's
pool allocator (the one with the internally allocated linked list in combination, which I don't think is actually originally attributed to him, but which I found in his book).
There is a lock free queue based on the moody camel one, but it uses the lofi static memory allocator to create worker specific pools of handles which are checked at the top of each thread's loop for
whether they can be deleted yet. It sacrifices a bit of cache coherency in task switching sections of execution for cache coherency within the tasks, as it is assumed more time will be spent in a particular
job than will be spent switching between them anyways.
Otherwise, it's very similar to the engine foundation presented in the 2016 gdc talk about the naughty dog engine, but with a bunch more metaprogramming with inspo from Odin Holmes and Andrei Alexandrescu.
