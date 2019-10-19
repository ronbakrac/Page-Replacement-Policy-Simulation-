# Page Swapping Simulator

This program takes a trace file of a program (a file that lists all virtual addresses that a program accesses in order, along with an indicator of being a read or write operation) and options of how many allocated frames to simulate in memory (RSS) and what page replacement algorithm to run. The program will output statistics of the trace for the specified algorithm and RSS that was set. These stats include the total number of events in the trace, number of disk reads, number of disk writes, and number of unique page accesses. A disk read will occur when there is a memory access but the page is not in RAM. A
disk write will occur when a page is evicted and if the page had been written to (known as being dirty). Once a page has been written to, it will remain dirty until written to disk. This program  assumed 4KB page sizes with 32-bit memory addresses.


The page replacement algorithms availible for this program are:

<strong>Least Recently Used (LRU):</strong> If RAM full, will swap the frame in memory that was least recently used.

<strong>First In First Out (FIFO):</strong> If RAM full, will swap the frame in memory that inserted first.

<strong>VMS Second Chance Page Replacement Policy:</strong> Each process has a normal FIFO queue but there exists 2 global queues, one 'dirty' and one 'clean'. If a process requests a page that is not in memory and memory is full, then this policy will check if there is anything in the global clean queue and dequeue from it first. If clean is empty then it will check if it can deqeue from dirty instead. If nothing is in dirty either then it will do the standard FIFO replacement for that processes queue. 


# More About VMS
The operating system used by Digital Equipment Corporation (DEC) for their minicomptuer, the VAX-11 (1970s), was called VAX/VMS. This replacement policy was one of their innovations. If you want to read more about the history of VMS and how the policy works then I strongly suggest reading chapter 23 of "Operating Systems: Three Easy Pieces" by Remzi H. Arpaci-Dusseau and Andrea C. Arpaci-Dusseau. The book is entirely free online by the authors at http://pages.cs.wisc.edu

# How To Compile

To compile the code needed to run, use the command:

`make memsim`

This will create the executable needed to run the program.

The program is run with the following arguments:

<strong>traceName</strong> - The name of a file containing a memory trace within the current directory.

<strong>nFrames</strong> - The number of physical frames to simulate memory with. 

<strong>Policy</strong> - Which paging policy to use. Choice of "lru" "fifo" "vms"

<strong>debug</strong> - If you want extra outputs, specify `debug` else use `quiet`

Then run with:

`./memsim <traceName> <nFrames> <Policy> <debug>`

You should see an output such as:

```Total memory frames: 2
Events in trace: 1000000
Total disk reads: 460912
Total disk writes: 232899
Unique Entries: 2852


