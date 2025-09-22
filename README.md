# Buffer-Manager
CS 525 Assignment 2

This is a simple buffer manager that allows interfacing with "disk" files.
First, create a page file using the storage manager.
When you have a page file, you can attach it to a buffer pool with your specifications by using the 'initBufferPool' function.
This buffer will allow you to more easily read and write to this page file. To access the page file, pin the page that you are looking for to the buffer using 'pinPage', and the index of that page within the file, providing this function with an area of memory that will be initialized to the contents of the page. You will be able to read from and write to this memory, and then commit your changes.
NOTE: Whenever you write to this page in memory, you must call 'markDirty' on that page, unless you have already done so.
When you are done making changes, you must call 'unpinPage' to make the manager know that you are no longer accessing it, allowing that page to be written back.
To commit your in-memory writes to the page file, call 'forceFlushBuffer', which will write back and dirty pages. (this is why you need to call markDirty.)
Remember that when you are done, you must manage the buffer by calling 'shutdownBuffer', which will de-allocate all of that buffer's memory.


# To the grader
To make the tests, run make test. This will put each executable into a file named 'testn', where n is the test number. Do note that I did not go out of my way to implement anything extra, so the second test will not be necessary.
