## System Programming 2020

A brief overview of the final project:

This project simulated a hospital program in which you could request patient records, the frequency of a disease in a country over a given time period, the top x age ranges that have been diseased, and other information. 

It was designed with a master program that created child processes and assigned them work through pipes. The work was to organize all of the data into data structures. As a result, a separate C program could run and act as a Server, communicating with the work processes via a port. There is also a separate multithreaded C program that acts as a client and requests data from the server (you choose how many threads to use in each request). 

In terms of design, I used hashtables and Avl tree structures in this project, as well as semaphores for synchronization.
I also, wrote a bash script that generated test subdirectories and input files for debugging and testing. 
