# lazycopier

Two program features:

A program to catch a segmentation fault with a signal handler and offer a more encouraging response to the user than simply “Segmentation Fault”.

A lazy copier that takes advantage of virtual memory to speed up copying large blocks of data. Instead of copying data immediately, it’ll create copies by mapping two virtual pages to point to the same memory, but mark that memory as read-only. If the program modifies either copy in the future it will perform the actual copy, but copies that are only read will complete much faster.
