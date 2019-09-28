Page Swapping Simulator

To compile the code needed to run, use the command:

`make memsim`

This will create the executable needed to run the program.

The program is run with the following arguments:

traceName - The name of a file containing a memory trace within the current directory.

nFrames - The number of physical frames to simulate memory with. 

Policy - Which paging policy to use. Choice of `lru` `fifo` `vms`

debug - If you want extra outputs, specify `debug` else use `quiet`

Then run with:

`./memsim <traceName> <nFrames> <Policy> <debug>`

You should see an output such as:

Total memory frames: 2
Events in trace: 1000000
Total disk reads: 460912
Total disk writes: 232899
Unique Entries: 2852