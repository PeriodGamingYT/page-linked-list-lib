# IN CONSTRUCTION!

page-linked-list is a method of allocation that doesn't involve malloc(). 
Instead, it will use VirtualAlloc()/mmap() for memory allocation.

A PageHeader is the first part of a page, describing how the page will be laid
out and its place in the linked list.