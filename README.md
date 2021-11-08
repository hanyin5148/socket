# socket
Update (Look here)
* Only need to compile the file test_ser.c
* >g++ -std=c++11 -pthread test_ser.c -o server 


test on rpi

* There are two versions server-client code  

* The first one :serverv1.c & clientv1.c   
  >Forked from [browny/simple_socket_example.c](https://gist.github.com/browny/5211329)  
  >the server sends the data(date+time)   
  +  How to use on the command line
  >g++ serverv1.c -o serverv1  
  >g++ clientv1.c -o clientv1  
  >./serverv1  
  >./client 127.0.0.1  
    

* The second one :server.c & client.c   
  >Modified by myself  
  >the server and the client can send the data to each other  
  +  How to use on the command line 
  >g++ -std=c++11 -pthread server.c -o server   
  >g++ -std=c++11 -pthread client.c -o client  
  >./server  
  >./client  
    
  +  Remark  
  >The IP needs to be defined in the code(client.c)  
