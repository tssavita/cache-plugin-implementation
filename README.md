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
