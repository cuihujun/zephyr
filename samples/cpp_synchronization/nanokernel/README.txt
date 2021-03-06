Title: C++ Synchronization

Description:
The sample project illustrates usage of pure virtual class, member
functions with different types of arguments, global objects constructor
invocation.

A simple application demonstrates basic sanity of the nanokernel.
The background task and a fiber take turns printing a greeting message to the
console, and use timers and semaphores to control the rate at which messages
are generated. This demonstrates that nanokernel scheduling, communication,
and timing are operating correctly.

--------------------------------------------------------------------------------

Building and Running Project:

This nanokernel project outputs to the console.  It can be built and executed
on QEMU as follows:

    make qemu

--------------------------------------------------------------------------------

Troubleshooting:

Problems caused by out-dated project information can be addressed by
issuing one of the following commands then rebuilding the project:

    make clean          # discard results of previous builds
                        # but keep existing configuration info
or
    make pristine       # discard results of previous builds
                        # and restore pre-defined configuration info

--------------------------------------------------------------------------------

Sample Output:

main: Hello World!
fiber_entry: Hello World!
main: Hello World!
fiber_entry: Hello World!
main: Hello World!
fiber_entry: Hello World!
main: Hello World!
fiber_entry: Hello World!
main: Hello World!
fiber_entry: Hello World!

<repeats endlessly>
