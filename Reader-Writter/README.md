# Reader Writter Problem


## Varition-1
---
 we need to ensure that , if multiple threads are waiting on queue, then the readers will be allowed to access the resource even when the writers are still on the queue.


![](1.png)

## Varition-2
---

if multiple threads are waiting on queue, then the writers will be allowed to access the resource even when the readers are still on the queue.

![](2.png)

## Variation-3
---
 both the readers and the writers will be given a equal weightage. 

 ![](3.png)



### Basic Blockchain using RW logic
