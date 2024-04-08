CONTRIBUTIONS

atticus:   
    - implemented all error handling
    - debugged issues with indexing
    - dubugged issues with segment faults
    - analyzed experiment 

matthew:
    - conducted experiment
    - implemented forking functionality
    - implemented memory mapping


REPORT

1: The following data points were the "real" times given by the "time" command

    Test run with threshold 2097152: 0m0.399s

    Test run with threshold 1048576: 0m0.245s

    Test run with threshold 524288: 0m0.156s

    Test run with threshold 262144: 0m0.133s

    Test run with threshold 131072: 0m0.152s

    Test run with threshold 65536: 0m0.146s

    Test run with threshold 32768: 0m0.149s

    Test run with threshold 16384: 0m0.159s


2: In order to explain why the times we saw may have occurred we must first aim to determine what parts of the sorting computations were being done in parallel, we need to
first determine what was being done without child processes. This is because anything being done within a child process
has the potential to be run in parallel to any other child process or parent process as the OS kernel determines how to
delegate and schedule such processes to available different CPU cores. The computation done in parallel is largely just within The
recursive merge sort calls and ultimately the qsort calls. This means that when we looks at the largest threshold value of 
2097152, we expected it to have the greatest run time as it utilizes parallel computation the least or not at all. Given such a threshold size
there are few child processes running this qsort of that size array in comparison to a threshold value of 524288 as a program
with a threshold value of 524288 has the potential to roughly 4x the number of child processes to sort the array compared to 2097152.
More child processes means more possibilities to take advantage of the multiple CPU cores and thus multiple parallel processes cutting time.
The reason we see the times flatten out as the threshold grows smaller than 524288 is likely due to the max delegation potential being reached
for CPU cores (all cores being used).