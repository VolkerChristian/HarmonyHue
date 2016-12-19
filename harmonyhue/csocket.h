#ifndef CSOCKET_H
#define CSOCKET_H


#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#endif

#include <stdio.h>
#include <string>
#define SUCCESS     0
#define FAILURE     1
#define TO_INFINITY   -1

class csocket 
{
public:

    enum SocketState 
    {
        CLOSED,
        CONNECTED,          
        ERRORED,
    };


    csocket();
    ~csocket(); 
	
	void            close();
    static int      resolveHost( const std::string& szRemoteHostName, struct hostent** pHostEnt );
    int             connect( const char* remoteHost, unsigned int remotePort );
    int             canRead( bool* readyToRead, float* remainingTime, float waitTime = TO_INFINITY );
    virtual int     read( char* pDataBuffer, unsigned int numBytesToRead, bool bReadAll );
    virtual int     write( const char* pDataBuffer, unsigned int numBytesToWrite );
    SocketState     getState( void ) const;
	int             getSocket( void ) const { return m_socket; }



private:
#ifdef WIN32
    SOCKET              m_socket;
#else
    int                 m_socket;
#endif
    struct sockaddr_in  m_localSocketAddr;
    struct sockaddr_in  m_remoteSocketAddr;

    SocketState         m_socketState;
    std::string         m_strRemoteHost;
    unsigned int        m_remotePort;
};

#endif // CSOCKET_H
