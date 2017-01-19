# Socket-Programming
Simple client server network using sockets

1. After compiling all files with makefile, run aws to turn on the server to receive connection from client.
2. Then run serverA, serverB, serverC in different terminals to turn on backend servers 
  that will perform calculations simultaneously.
3. Run client with the function to run and the file of numbers that need to be processed across the servers.
4. The AWS server will receive and read the file to distribute the numbers across the 3 backend servers 
5. Each backend server will perform the desired calculation and send its result back to the AWS server.
6. The AWS server will wait for and compile the results of each server before sending the final result to the client.
