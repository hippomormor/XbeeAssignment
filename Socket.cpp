/*
 * Socket.cpp
 */

#include "socket.h"
#include "timer.h"
#include "errno.h"
#include "arpa/inet.h"
#include <unistd.h>


Socket::Socket()
{
  my_instance = this;
  rx_socket_desc = -1;
  tx_socket_desc = -1;
  tx_is_connected = 0;
}

Socket::~Socket()
{
  DestroyTx();
  DestroyRx();
}

int Socket::InitRx(int input_port, int package_size, Callback callback_arg)
{
  int c,i;

  // store package size locally
  this->package_size = package_size;
  this->callback = callback_arg;

  /* create listen-socket */
  rx_socket_desc= socket(AF_INET , SOCK_STREAM , 0);
  if (rx_socket_desc == -1)
  {
    printf("Could not create socket");
    return (0);
  }

  /* Enable address reuse */
  c= 1;

  if (setsockopt(rx_socket_desc, SOL_SOCKET, SO_REUSEADDR, &c, sizeof(c)))
  {
    printf("\n\rsetsockopt failed. Error");
    return (0);
  }

  fcntl(rx_socket_desc, F_SETFL, O_NONBLOCK);
  fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

  /* Prepare the sockaddr_in structure */
  rx_server.sin_family= AF_INET;
  rx_server.sin_addr.s_addr= INADDR_ANY;
  rx_server.sin_port= htons(input_port);

  /* Bind */
  if (bind(rx_socket_desc,(struct sockaddr *)&rx_server , sizeof(rx_server)) < 0)
  {
    //print the error message
    printf("\n\rbind failed. Error");
    return (0);
  }

  /* Listen */
  listen(rx_socket_desc, 3);

  printf("\n\rRx: Listening @ port %d (%d)", input_port, rx_socket_desc);
  if (this->callback)
  {
    printf("\n\rRx: Callback defined");
  }
  fflush(stdout);

  for (i=0;i<MAX_CLIENTS;i++)
  {
    ClientList[i].Flag = 0;
  }

  // create thread to handle incoming data
  pthread_create(&pthread_sock_main_process, NULL, this->socket_listen_main_process, this);

  return (1);
}

int Socket::RxListen(void)
{
  struct sockaddr_in client;
  int c;
  int i,k;
  int ThisSock;
  int Len;
  int read_size;
  struct timeval timeout;

  c = sizeof(client);

  /* start the infinite loop */
  while (1)
  {
    timeout.tv_sec = 0;
    timeout.tv_usec= 1000;
    select(0,0,0,0,&timeout);
    //usleep(10000);
    ThisSock= accept(rx_socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (ThisSock >= 0)
    {
      /* new socket established */
      /* find available entry in list */
      struct _ClientList *pCurClient;
      for (pCurClient=0,i=0;i<MAX_CLIENTS;i++)
      {
        if (ClientList[i].Flag == 0)
        {
          pCurClient = &(ClientList[i]);
          break;
        }
      }

      if (!pCurClient) exit(0);
      pCurClient->Id = ThisSock;
      sprintf(pCurClient->IpAdr, "%d.%d.%d.%d",
        (unsigned char)(client.sin_addr.s_addr),
        (unsigned char)(client.sin_addr.s_addr >> 8),
        (unsigned char)(client.sin_addr.s_addr >> 16),
        (unsigned char)(client.sin_addr.s_addr >> 24));
      pCurClient->pInBuf = (char*)malloc(65535);
      pCurClient->pReadyBuf = (char*)malloc(65535);
      if (pCurClient->pInBuf && pCurClient->pReadyBuf) {
        memset(pCurClient->pInBuf, 0, 65535);
        memset(pCurClient->pReadyBuf, 0, 65535);
        pCurClient->pWrite = pCurClient->pInBuf;
        pCurClient->pEnd = pCurClient->pInBuf + 65535;
        pCurClient->pReady = pCurClient->pReadyBuf;
        //printf("\n\rSocket established from %s", pCurClient->IpAdr);
        printf("\n\rRx: Socket established on port %d from %s:%d (%d)",
         ntohs(rx_server.sin_port),
         pCurClient->IpAdr, client.sin_port,
         rx_socket_desc);
        fflush(stdout);
        pCurClient->RawFlag = 1;
        pCurClient->ReadyFlag = 0;
      }
    }

    /* read data for all open connections */
    for (i=0;i<MAX_CLIENTS;i++)
    {
      if (ClientList[i].RawFlag == 1)
      {
        Len = recv(ClientList[i].Id, ClientList[i].pWrite, (size_t)(ClientList[i].pEnd-ClientList[i].pWrite), MSG_DONTWAIT);
        if (Len <= 0)
        {
          // nothing read
        }
        else
        {
          ClientList[i].pWrite += Len;
          //*(ClientList[i].pInputBufWritePos) = 0;

        }
      }
    }

    /* preanalyze data (look for sufficient chunk of data) */
    for (i=0;i<MAX_CLIENTS;i++)
    {
      if (ClientList[i].RawFlag == 1 && ClientList[i].ReadyFlag == 0)
      {
        int Remain;
        int NumPackages, NumFit;
        int RawSize, PackSize;
        char *pStart;

        RawSize = (ClientList[i].pWrite - ClientList[i].pInBuf);
        NumPackages = (int)(RawSize/package_size);
        NumFit = (int)(65535 / package_size);
        if (NumFit < NumPackages) NumPackages = NumFit;
        PackSize = NumPackages * package_size;

        if (NumPackages)
        {
          //printf("\n\r%d %d %d", RawSize, (int)(RawSize/package_size), NumFit);
          //fflush(stdout);
          /* move data to other buffer (ready) */
          memcpy(ClientList[i].pReady, ClientList[i].pInBuf, PackSize);
          //printf("\n\rMoving data to %08x", ClientList[i].pReady);
          ClientList[i].pReady += PackSize;
          //printf("\n\rSize of ready buffer: %d", (int)(ClientList[i].pReady - ClientList[i].Ready));

          pStart = ClientList[i].pInBuf + PackSize;
          Remain = (int)(ClientList[i].pWrite - pStart);
          if (Remain > 0)
          {
            //printf("\n\rMoving %d bytes", Remain);
            memmove(ClientList[i].pInBuf, pStart, Remain);
          }

          ClientList[i].pWrite = ClientList[i].pInBuf + Remain;

          //printf("\n\rRaw buffer: %08x %d", ClientList[i].pWrite, (int)(ClientList[i].pWrite - ClientList[i].InBuf));
          //printf("\n\rReady buffer: %d", (int)(ClientList[i].pReady - ClientList[i].Ready));

          //fflush(stdout);
        }
        if (ClientList[i].pReady > ClientList[i].pReadyBuf)
        {
          ClientList[i].ReadyFlag = 1;
          if (this->callback)
          {
            int Size = (int)(ClientList[i].pReady - ClientList[i].pReadyBuf);
            //printf("Invoke callback for Rx"); fflush(stdout);
            for (k=0;k<Size;k+=package_size)
            {
              callback((void*)(ClientList[i].pReadyBuf+k));
            }
            ClientList[i].ReadyFlag = 0;
            ClientList[i].pReady = ClientList[i].pReadyBuf;
          }
          else
          {
            // no callback defined. Application is responsible for
            // calling Socket::Rx in order to fetch data
          }
        }

      }
    }
  }
}

int Socket::Rx(void *pDat)
{
  char *pTmp;
  int i;

  if (rx_socket_desc == -1) return (0);

  for (i=0;i<MAX_CLIENTS;i++)
  {
    if (ClientList[i].ReadyFlag == 1)
    {
      int Size = (int)(ClientList[i].pReady - ClientList[i].pReadyBuf);
      //printf("Client %d ready with %d bytes of data", i, Size); fflush(stdout);
      memcpy(pDat, ClientList[i].pReadyBuf, package_size);

      int Remain = Size - package_size;
      if (Remain)
      {
        memmove(ClientList[i].pReadyBuf, ClientList[i].pReadyBuf+package_size, Remain);
        ClientList[i].pReady = ClientList[i].pReadyBuf + Remain;
      }
      else
      {
        // now ready for input again
        ClientList[i].ReadyFlag = 0;
        ClientList[i].pReady = ClientList[i].pReadyBuf;
      }
      return (1);
    }
  }
  return (0);
}


int Socket::InitTx(char* ip_address, int Port)
{
  char ip_adr[32];

  if (!strcmp(ip_address, "localhost"))
    strcpy(ip_adr, "127.0.0.1");
  else
    strcpy(ip_adr, ip_address);

  tx_connect_port = Port;
  inet_aton(ip_adr, &tx_server.sin_addr);
  tx_server.sin_family = AF_INET;
  tx_server.sin_port = htons(Port);
  // create thread to (re)connect tx connection
  pthread_create(&pthread_reconnect, NULL, this->socket_reconnect_tx, this);

  // wait max 2 secs in order to establish connection
  Timer LocalTimer(2);
  for (;!LocalTimer.Timeout();) {
    if (tx_is_connected) break;
  }

  return (1);
}

int Socket::TxReconnect(int Port)
{
  int ok;
  char buf;
  int err;
  char *pIpadr;

  pIpadr = inet_ntoa(tx_server.sin_addr);

  printf("\n\rTx: Trying to (re)connect to %s:%d", pIpadr, tx_connect_port);
  fflush(stdout);
  while (1)
  {
    //sched_yield();
    usleep(10);

    if (!tx_is_connected)
    {
      //Create Tx socket
      tx_socket_desc = socket(AF_INET , SOCK_STREAM , 0);
      if (tx_socket_desc == -1)
      {
        printf("\n\rCould not create socket");
        fflush(stdout);
      }
      //printf("Attempt to connect to %s:%d", pIpadr, Port);
      ok = connect(tx_socket_desc, (struct sockaddr*)&tx_server, sizeof(tx_server));
      if (ok == 0)
      {
        printf("\n\rTx: Connect to %s:%d success (%d)", pIpadr, Port, tx_socket_desc);
        fflush(stdout);
        tx_is_connected = 1;
      }
      else
      {
        //printf("Tx: Connect to %s:%d failed (%d)\n\r", pIpadr, Port, errno);
        //fflush(stdout);
        close(tx_socket_desc);
        tx_is_connected = 0;
      }
    }
    else
    {
      err = recv(tx_socket_desc, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
      if (!err)
      {
        printf("\n\rTx: WARNING: Connection to %s:%d is lost", pIpadr, Port);
        fflush(stdout);
        close(tx_socket_desc);
        tx_is_connected = 0;
      }
    }
  }
}

int Socket::Tx(void* pDat, int Len)
{
  int rv = 0;
  if (tx_socket_desc != -1 && tx_is_connected)
  {
    rv = send(tx_socket_desc , pDat, Len, MSG_DONTWAIT);
  }
  else
  {
    printf("\n\rTx: ERROR: Can't send %d bytes (port %d) %d %d", Len, tx_connect_port, tx_socket_desc, tx_is_connected); fflush(stdout);
  }
  return (rv);
}

int Socket::DestroyTx(void)
{
  if (tx_socket_desc != -1)
    close(tx_socket_desc);
  tx_is_connected = 0;
  tx_socket_desc = -1;
}

int Socket::DestroyRx(void)
{
  int i;

  if (rx_socket_desc != -1)
    close(tx_socket_desc);

  for (i=0;i<MAX_CLIENTS;i++)
  {
    if (ClientList[i].pInBuf) free(ClientList[i].pInBuf);
    if (ClientList[i].pReadyBuf) free(ClientList[i].pReadyBuf);
    memset(&(ClientList[i]), 0, sizeof(struct _ClientList));
  }
  rx_socket_desc = -1;
}

/* ideas: callback for Tx done, */





