# OS_ALL_CONN_NETWORK


OS
M&S, HW3

***PARTA

# Chat:- 

    # RUN

    The client side: stnc -c IP PORT
    The server side:stnc -s PORT
    the communication done using IPv4 TCP protocol

    ![Screenshot from 2023-05-12 11-50-06](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/8fa4fa61-a016-4f60-9aa3-2d33916ac626)


    ![Screenshot from 2023-05-12 11-49-41](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/2a26856c-277c-4c77-81bc-03fc183684e9)




***///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




***PARTB

# Clients:- 

    # RUN
    
  <div> 
  
    communications styles are:
    tcp/udp ipv4/ipv6 (4 variants)
    mmap a file. Named pipe (2 variants)
    Unix Domain Socket (UDS) :stream and datagram (2 variants)
    usage:
    The client side: stnc -c IP PORT -p <type> <param>
    -p will indicate to perform the test
    <type> will be the communication types: so it can be ipv4,ipv6,mmap,pipe,uds
    <param> will be a parameter for the type. It can be udp/tcp or dgram/stream or file name:
 
    </div> 

    
<a>
    
void c_tcp_ipv4_channel(char* address[]);
void c_tcp_ipv6_channel(char* address[]);
void c_udp_ipv4_channel(char* address[]);
void c_uds_stream_channel(char* address[]);
void c_uds_dgram_channel(char* address[]);
void c_udp_ipv6_channel(char* address[]);
void c_pipeline_channel(char* address[]);
void c_mmap_channel(char* address[]);

</a>





# Servers:- 

**Each function have her work to connect with specifc (type) and (param) , in the final it's return the time that's get to do the job:- 

<a> 

long s_tcp_ipv4_channel();
long s_tcp_ipv6_channel();
long s_udp_ipv4_channel();
long s_uds_stream_channel();
long s_uds_dgram_channel();
long s_udp_ipv6_channel();
long s_pipeline_channel();
long s_mmap_channel();

</a>


  <div> Some Pictures for run code (PARTB) </div> 

***///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  
  
  <div>  udp_ipv4 </div> 
  
  ![Screenshot from 2023-05-12 12-05-08](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/62f69e22-9de3-4bd7-a6c7-4cd4d8f134df)

  
  ![Screenshot from 2023-05-12 12-04-49](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/42285532-b7f9-45bb-9484-b4eff841cf4b)


   <div>  udp_ipv6 </div> 
    
  ![Screenshot from 2023-05-12 12-07-24](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/a5dcb6ff-86c4-497b-b9c7-da641a976005)

  ![Screenshot from 2023-05-12 12-07-10](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/984bca08-b5de-4b5a-88c5-e60f70a14692)

    
    
    
 ***///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  
   <div>  tcp_ipv6 </div> 

  ![Screenshot from 2023-05-12 12-12-43](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/0af77187-df9a-4282-b223-0e0024fcd10f)

  
  ![Screenshot from 2023-05-12 12-12-29](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/bb4feb0b-8d10-43e1-bac2-0010c4c6acd6)

  
   <div>  tcp_ipv4 </div> 

  
  ![Screenshot from 2023-05-12 12-13-55](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/58806f8f-99c4-42e6-8ac4-b63fbecb8ccb)


  ![Screenshot from 2023-05-12 12-13-45](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/39633657-9783-4f9f-bc4d-2b32b168011a)

  
  
 ***///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

     <div>  mmap </div> 

  
  ![Screenshot from 2023-05-12 12-18-00](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/5b53d727-7ba3-4dbe-86b9-2fd1f257fb9e)
  
  
  ![Screenshot from 2023-05-12 12-17-48](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/c4dd1e7e-abc0-40c1-a9d1-4ea698ca85d0)
  
  
   ***///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

       <div>  pipe </div> 
  
  ![Screenshot from 2023-05-12 12-20-09](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/017bc8fd-7dda-42c0-b022-d65e703a43e4)
  
  
  ![Screenshot from 2023-05-12 12-19-57](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/096d3f46-6a5b-4562-821e-44255b42b82b)

  
  
  <div> Then will get a new file with the data :- </div> 
  
  
  ![Screenshot from 2023-05-12 12-21-15](https://github.com/Mohanadsfe/OS_ALL_CONN_NETWORK/assets/92846018/31afdaff-8ee0-4d9b-8ae4-908f7b38eb5b)

  
  
  

  
  


  
  


