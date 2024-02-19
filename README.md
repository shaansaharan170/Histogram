## Shaan Saharan

## Project Overview
This project implements a parallel program in C that calculates English alphabet histograms from multiple text files using a parent-child process model with IPC (Inter-process communication) through pipes and signal handling.

## Description
The program takes a list of file names as input through command line arguments and generates a new set of files containing the English alphabet histograms for each input file. Each child process calculates the histogram for a given file and sends the data back to the parent process, which then saves the histogram to a file named filePID.hist, where PID is the Process ID of the corresponding child.

## Features
- **Parent Process:** Accepts multiple file names as command line arguments and forks a child process for each file to calculate the histogram.
- **Child Process:** Computes the histogram for the assigned file and sends it back to the parent process through a pipe.
- **Signal Handling:** Uses SIGCHLD to handle terminating child processes asynchronously, ensuring data is read-only from children that terminate normally.
- **Error Handling:** Properly handles file opening failures and abnormal child terminations.

## Usage
1. Compile the program using the provided Makefile:

- make

2. Run the program with a list of text files as arguments:

- ./Histogram file1.txt file2.txt ...etc

- To simulate a signal interruption, use SIG as an argument:

- ./Histogram file1.txt SIG file2.txt ...etc

3. The Makefile targets are:
- Histogram: Compiles the main executable.
- clean: Removes the executable and temporary files.

## Requirements
- POSIX-compliant system for signal handling and IPC.
- C compiler supporting C11 standard.

## Known Issues
- The program assumes text files use the 26-letter English alphabet. Non-alphabetic characters are ignored.
