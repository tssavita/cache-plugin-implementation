Monkey Caching Plugin
=====================

This caching plugin has been written with the aim to improve the already high performance of Monkey HTTP Server and make it even better. It caches the static files handled by the server.  

Installation
============

$ git clone https://github.com/monkey/monkey.git  
$ cd monkey  
$ git clone https://github.com/tssavita/cache-plugin-implementation.git plugins/cache/  
$ ./configure --malloc-libc --debug --enable-plugins=cache  
$ make  
$ bin/monkey

Status
======

The caching plugin has been found to be improving the performance of the server by approximately 13 percent. However, the user interface for the plugin still needs to be made changes to, and is not yet up and running. 

Highlight
=========

The highlight feature of this plugin is it uses the system call 'mmap' to map onto the memory, the file that is to be cached. Mmap is useful in cases where multiple processes access the same files. For more details about mmap, one could refer to the man page for the same.

User Interface
==============

To get the JSON output, you could browse to 'http://localhost:2001/cache/stats'. To view the statistics graphically, once you have the server running, you could just browse to 'http://localhost:2001/cache/index.html'. Here you can have a look at the real time graph that shows you the number of requests that the plugin handles. You also have the option of completely clearing the cache from the user interface and of viewing the number of hits and misses that the plugin encountered.

The statistics get updated at an interval of 60 seconds.
