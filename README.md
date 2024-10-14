#######################################################################################################
README for Torrent File Exchange Assignment
Introduction
This project demonstrates socket programming in C by implementing a simplified torrent file exchange scenario. The server acts as a seeder, allowing clients to download files. The client connects to the server to request and download files in chunks, mimicking torrent-like behavior.

Steps for the project
- shared_files  # Directory containing files for download
  - srv5200.c  # Server program
  - cli2236.c  # Client program

Prerequisites
C compiler (e.g., GCC)
Basic understanding of socket programming and C programming language
Make sure you have a directory named shared_files/ containing at least 50 files, each with a size of 10MB or more.
Compilation
To compile the server and client programs, navigate to the src directory and run the following commands:

bash
Copy code
gcc srv5200.c -o server 
gcc cli2236.c -o client


Run the server:./server
The server will start listening for incoming connections.
Running the Client
Open a new terminal window.

Run the client and connect to the server:./client
port number:5200
################################################################
Define file directory to store file in server side named server_store.
Define file directory to save recieve files named client_store

The client will request the list of available files from the server.
The user can select a file from the list to download.
The client will display a progress bar as the file downloads.
Upon completion, the downloaded file will be saved in the original format.
Logging
The server logs all client connections and file transfers in a log file named log_srv The log includes:

Client IP address and port
Connection date and time
Requested file name
Transfer status (successful or failed)
Error Handling
Both the server and client programs include error handling for:

Socket creation, binding, listening, and connection errors.
File transfer errors.
##############################Assumptions###################
The server is running before the client attempts to connect.
The files in the shared_files/ directory are accessible and readable by the server.
Challenges Encountered
Handling multiple client connections simultaneously required the implementation of multi-processing.
Properly managing file transfers in chunks while ensuring data integrity.
#####Test Cases#######################
Successful connection and file transfer.
Handling of invalid file requests.
Testing simultaneous connections from multiple clients.
