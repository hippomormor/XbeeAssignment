/*
 * Socket.h
 *
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#ifdef __cplusplus
#include <stdlib.h>
#include <stdio.h>
#include "fcntl.h"
#include <memory.h>
#include <functional>


#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>  // sockaddr_in, htons

#define MAX_CLIENTS 10

struct _ClientList 
{
  int Id;
  char IpAdr[20];
  char *pEnd;
  char *pWrite;  
  char *pInBuf;
  char *pReadyBuf;
  char *pReady;
  int Flag; // 0=not in use; 1=socket established; 2=data recieved (debug-chunks ready); 3=data in progress of being submitted to file
  int RawFlag; // 0=not allowed to write; 1=allowed to write
  int ReadyFlag;
};

class Socket
{
public:
  typedef std::function<int(void*)> Callback;
    
  Socket();
  ~Socket();
  int InitRx(int input_port, int package_size, Callback callback_arg);
  int Rx(void *pDat);
  int InitTx(char *ip_address, int Port);
  int Tx(void *pDat, int Len);
  int DestroyTx(void);
  int DestroyRx(void);
    
private:
  Socket* my_instance;
  // Rx
  int rx_socket_desc;
  struct sockaddr_in rx_server;
  struct _ClientList ClientList[MAX_CLIENTS];
  int package_size; // for Rx data  
  pthread_t pthread_sock_main_process; 
  pthread_t pthread_reconnect; 
  Callback callback;  
  
  static void *socket_listen_main_process(void *This)
  {
    ((Socket*)This)->RxListen();
  }
  int RxListen(void);
  
  // Tx
  int tx_socket_desc;
  int tx_connect_port;
  struct sockaddr_in tx_server;
  int tx_is_connected;

  static void *socket_reconnect_tx(void *This) 
  {
    //printf("Thread started: %p %d\n\r", (Socket*)This, ((Socket*)This)->tx_connect_port);
    ((Socket*)This)->TxReconnect(((Socket*)This)->tx_connect_port);    
  }
  int TxReconnect(int Port);
  

};
#endif

#ifdef __cplusplus
#define extern_C extern "C"
#else
#define extern_C
#endif


#endif /* SOCKET_H_ */
