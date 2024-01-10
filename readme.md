# README for Miniature FTP System

## Overview
This README provides guidance for using the `myftp` and `myftpserve` programs, developed as a part of the Systems Programming (CS 360) final project at Washington State University Vancouver. These programs constitute a miniature FTP system, designed for a Linux OS environment and utilizing Unix TCP/IP sockets for communication.

## Program Descriptions
- **`myftp`**: The client program. It is initiated from the command line with the program name `myftp` followed by the host's name.
- **`myftpserve`**: The server program. It operates without login requirements and logs all activities and errors.

## User Commands
- **`exit`**: To terminate the client session.
- **`cd`**, **`rcd`**: To change the directory on the client (`cd`) and server (`rcd`).
- **`ls`**, **`rls`**: To list files in the current directory on the client (`ls`) and server (`rls`).
- **`get`**: To download a file from the server.
- **`show`**: To display a file from the server on the client.
- **`put`**: To upload a file to the server.

## Installation and Execution
- Compile the programs using the provided Makefile.
- Run `myftpserve` on the server machine.
- On the client machine, run `myftp` followed by the server's hostname.

## Debugging and Error Handling
- Detailed error messages are provided for various scenarios.
- Server logs all activities and encountered errors for troubleshooting.
