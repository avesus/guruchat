#define _WIN32_WINNT 0x0501
#define _WINSOCKAPI_
# include <windows.h>
#include <ws2tcpip.h>
# include <winsock.h>
# include <winsock2.h>

extern "C"
{
# include "./../thirdparty/codec2/src/codec2.h"
# include "./../thirdparty/opus/include/opus.h"
}

# include "audio_capture.h"
# include <malloc.h>
# include <intrin.h>

# include <stdio.h>

# include "./../nanosec.h"
# include <process.h>

# pragma comment(lib, "ws2_32")

void StartVideoCapture();
void CaptureAudio();

extern int compressedVideoWidth;
extern int compressedVideoHeight;// = compressedVideoWidth * 9 / 16 / 16 * 16;

// ��������:
// 1. ���� ���� ���������� IPv6, �� ���� ����������� ��������,
// NAT �� ����� ������. ����� ������ IPv6 �� IPv6 ��������� �� �����.
// �� ��� ��� �������� ����� �����������, � ������� ���� ������������
// (������, � �������, ��������� ����), �� IPv6 ������� ����� � ������
// � ����� ����������������� ��������� ����������� �������.
// ���������� ��������� ���, � ������� IPv4 ����� �� ������������.
// ����� ������ ����� �����:
// OS6, LAN6, INET6

// �� ����������� IPv6:
// OS4, LAN4, INET4_NAT_REVERSE, INET4_NAT_STABLE, INET4

// ���� ���� IPv4, �� ��� ��������� �� IPv6, �� IPv6 ����� ������ ������������,
// �.�., LAN4 - ������� IPv4, ���� ���� ���� ��� � IPv6.
// LAN6 - ������ IPv6, ����� IPv4 �� ��������� ������.
// ����� ������ �����:
// OS4, LAN4, LAN6, INET4_NAT_REVERSE, INET4_NAT_STABLE, INET4, INET6, INET46

# if 0
// ������������� ������� ��������, "������� �����������"
enum ServerLevel
{
  // ������� �������� �����������, ���� �� ��� ��������� ��� ���������.
  // ����� �������������� ��� �������� ����� �� ���������� � ����������� ����������,
  // ��������, ����������� � ����������.
  // ��������� ����� �� IPv6 � IPv4.
  SL_OS = 2,

  // ��������� ���� ���������� � �� �� ����������� �����.
  // ��� ��� ����� ���� ��������� ������� ��������� � �� ����� �� ��� ����� ���� ������
  // IPv4, � �� ������ - IPv6, � ��������� ������ ����������� ����� ��������
  // �� ����������������.
  // ��������� �� ������ ����� �� �������� �� ������ ������� ��������.
  // ��������� ������� ����� ��������� IP-������� (� �������)
  // ��� ������� �� ��������� ����.
  SL_LAN = 3,

  // ������� ����������� � ��������� ����� NAT, �� ��� ������ �� ��������
  // �� ����� - ������� ���� �� ����� ��������.
  // ��� ������ ����� �������� ������ ����� ������ ����, ��������� ��� �������,
  // ��� � ��������. ��������� ������ ����� �������� � ���� ������� ������,
  // �, ����� ������ ����� ������������, �� �� ������� ����� �������
  // ���������� � ����, � ��������� ������, ������� ��� ����������,
  // ��� ����������� � �������. ��� ����������������� ����� �����������,
  // ����� � ������� ��� ������� ������� SL_I4_NAT � ����.
  SL_I4_REVERSE = 4,

  // �������� �������� �� "��������" IP-������. �� ����� �������� ��� ������,
  // ��� ��� ������� ���� �� ��������. ��� ��� ����������� ��������� STUN-��������
  // ������������ ������ IPv4, ������ �� NAT'�� ����� �������������, ��� � ��� �������
  // IPv4-����� ������. �� ����� ��������, ����� � ��� ��������� ����.
  SL_I4_NAT = 4,

  // Real external IP-address. IPv6 only.
  SL_I6_REAL = ,

  // Real external IP-address. IPv4 only.
  SL_I4_REAL = ,

  // Real external IP-address. Both IPv4 and IPv6.
  SL_I46_REAL = 5
};
# endif

void DetermineExternalAddress()
{
  WSADATA data = {0};
  ::WSAStartup(MAKEWORD(2, 2), & data );

  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  sockaddr_in cltAddr = {0};
  cltAddr.sin_family = AF_INET;
  cltAddr.sin_port = htons(5555);

  bind(sock, (struct sockaddr *)&cltAddr, sizeof(cltAddr));

  sockaddr_in serverAddr = {0};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.S_un.S_addr = inet_addr("23.21.150.121");//"212.79.111.155");
  //serverAddr.sin_addr.S_un.S_addr = inet_addr("173.194.69.127");
  //serverAddr.sin_addr.S_un.S_addr = inet_addr("74.125.23.127");
  serverAddr.sin_port = htons(3478);
  //serverAddr.sin_port = htons(19302);

# pragma pack(push,1)
  struct StunRequest
  {
    uint16_t msgType;
    uint16_t payloadLen;
    uint32_t magic;
    uint8_t transaction[12];
  };
# pragma pack(pop)

  StunRequest r = {0};
  r.msgType = 0x0100;
  r.payloadLen = 0;
  r.magic = 0x42A41221;
  r.transaction[0] = 5;
  r.transaction[5] = 7;

  int sentBytes = sendto(sock, (char*)&r, sizeof(r), 0,
    (sockaddr*)&serverAddr, sizeof(serverAddr));

  char recvBuffer[ 1280 ] = {0};

  sockaddr_in from = {0};
  int fromLen = sizeof(from);
  int nrecvd = recvfrom(sock, recvBuffer, sizeof(recvBuffer),
    0, (sockaddr*)&from, &fromLen);

  uint16_t externalPort = ntohs(*((uint16_t*)&recvBuffer[26])) ^ 0x2112;
  uint32_t externalIp = ntohl(*((uint32_t*)&recvBuffer[28])) ^ 0x2112A442;

  char recvLocal[ 1280 ] = {0};
/*
  sockaddr_in fromLocal = {0};
  int fromLenLoc = sizeof(fromLocal);
  int nrecvdLoc = recvfrom(sock, recvLocal, sizeof(recvLocal),
    0, (sockaddr*)&fromLoc, &fromLenLoc);
*/
  //StunRequest* response = (StunRequest*)recvBuffer;

}

extern void calculate();

void SendRecv()
{
  // ���� ���� ��� ���������, ��������� 1 �����
  // ���� ����� ����� ������ ���� ��� ����������, ����� ���������� ���,
  // ����� ������������ ��������.
  // ���� ������ ��������� � ������ ����������, ����
  // ���������� � ������ ��� �������, ��� ����, ��� ����������.

}

int GetError()
{
# ifdef _MSC_VER
  return GetLastError();
# endif
}

class CPrecisionWaitable
{
public:
  CPrecisionWaitable() :
    m_hTimer(::CreateWaitableTimerW(0, TRUE, 0))
  {
  }

  void Wait(UINT64 microseconds)
  {
    LARGE_INTEGER waitTime = {0};
    waitTime.QuadPart = microseconds * -10;
    ::SetWaitableTimer(m_hTimer, &waitTime, 0, 0, 0, FALSE);
    ::WaitForSingleObject(m_hTimer, INFINITE);
  }

private:
  HANDLE m_hTimer;
};

const int packetSize = 1024;

uint16_t recvOnPort = 8888;
uint16_t sendToPort = 8888;

char anotherIp[64] = {0};
char multicastDestIp[64] = {0};
char multicastSourceIp[64] = {0};

unsigned int __stdcall SenderThread(void* param)
{
  //CPrecisionWaitable waiter;

  int sock = (int)param;
  char sendBuffer[ packetSize ] = {0};
/*
  sockaddr_in serverAddr = {0};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.S_un.S_addr = inet_addr(anotherIp);//"82.199.103.175");
  serverAddr.sin_port = htons(sendToPort);
*/
  sockaddr_in mcAddr = {0};
  mcAddr.sin_family = AF_INET;
  mcAddr.sin_addr.S_un.S_addr = inet_addr(multicastDestIp);
  mcAddr.sin_port = htons(sendToPort);

  UINT64 frameNumber = 0;

  while(true)
  {
    *((UINT64*)sendBuffer) = Nanosec::Now();
    *((UINT64*) (& sendBuffer[ 8 ])) = frameNumber++;

    int nPacketSize = sizeof(sendBuffer);

    int sentBytes = sendto(sock, sendBuffer, nPacketSize, 0, (sockaddr*)&mcAddr, sizeof(mcAddr));
    //int sentBytes = sendto(sock, sendBuffer, nPacketSize, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if(sentBytes < 0)
    {
      int err = GetError();
      if(WSAEADDRNOTAVAIL == err)
      {
        ;
      }
    }
    //waiter.Wait(1);
    Sleep(100);
  }
}

int Receiver()
{
  WSADATA data = {0};
  ::WSAStartup(MAKEWORD(2, 2), & data );

  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  //int reuseEnabled = 1;
  //int res01 = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseEnabled, sizeof(reuseEnabled));

  sockaddr_in serverAddr = {0};
  serverAddr.sin_family = AF_INET;
  //serverAddr.sin_port = htons(recvOnPort);
  //serverAddr.sin_addr.S_un.S_addr = inet_addr("192.168.137.14");

  // Assign SOURCE IP for socket (INADDR_ANY - automatic)
  bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
  int nLen = sizeof(serverAddr);
  ::getsockname(sock, (sockaddr*)&serverAddr, &nLen);
  recvOnPort = ntohs(serverAddr.sin_port);

  printf("My port: %d\n", recvOnPort);

/*
  sockaddr_in mcAddr = {0};
  mcAddr.sin_family = AF_INET;
  mcAddr.sin_addr.S_un.S_addr = inet_addr("192.168.137.93");
  mcAddr.sin_port = htons(sendToPort);
  // Server
  int res1 = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (const char*)&mcAddr, sizeof(mcAddr));*/

  //int broadcastEnabled = 1;
  //int res0 = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcastEnabled, sizeof(broadcastEnabled));

  // Client
  ip_mreq mreq = {0};
  mreq.imr_multiaddr.S_un.S_addr = inet_addr(multicastSourceIp);
  mreq.imr_interface.S_un.S_addr = 0;//inet_addr("192.168.137.14");
  int res2 = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));

  _beginthreadex(0, 0, SenderThread, (void*)sock, 0, 0);

  char recvBuffer[ packetSize ] = {0};

  UINT64 prevTime = Nanosec::Now();
  UINT64 currTime = Nanosec::Now();

  UINT64 currReceivedFrameNumber = 0;

  //getsockopt(sock, 

  //Sleep(INFINITE);

  while(1)
  {
    sockaddr_in from = {0};
    int fromLen = sizeof(from);
    int nrecvd = recvfrom(sock, recvBuffer, sizeof(recvBuffer),
      0, (sockaddr*)&from, &fromLen);
    if (from.sin_addr.S_un.S_addr == 0x5d89a8c0)
      continue;
    if( nrecvd == -1)
      continue;

    UINT64 distTime = *((UINT64*)recvBuffer);

    UINT64 prevReceivedFrameNumber = currReceivedFrameNumber;

    currReceivedFrameNumber = * ((UINT64*) (& recvBuffer[ 8 ]));

  }

  return 0; 
}


//int WINAPI wWinMain(HINSTANCE, HINSTANCE, wchar_t*, int)
int main()
{
  char anotherPort[16] = {0};
  char myPort[16] = {0};
  printf("Recv multicast from (ex. 239.192.100.1):\n");
  scanf_s("%63s", multicastSourceIp, 64);
  printf("Broadcast to (ex. 239.192.100.2):\n");
  scanf_s("%63s", multicastDestIp, 64);
  printf("Another IP:\n");
  scanf_s("%63s", anotherIp, 64);

  printf("Another port:\n");
  scanf_s("%15s", anotherPort, 16);
  sendToPort = atoi(anotherPort);

  Receiver();

  calculate();
  return 0;
  // Max. bitrate: 512000 for audio, 20 000 000 for video.
  // ~30 MBit/s

  //DetermineExternalAddress();

  //return 0;

  //CaptureAudio();

  short bufIn[320];


  ///////////////////
  // OPUS (SILK+CELT)
  int err = 0;
  OpusEncoder* opus = opus_encoder_create(8000, 1,
    OPUS_APPLICATION_RESTRICTED_LOWDELAY, &err);//OPUS_APPLICATION_VOIP, &err);

  int ok = opus_encoder_ctl(opus, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
  ok = opus_encoder_ctl(opus, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_NARROWBAND));
  ok = opus_encoder_ctl(opus, OPUS_SET_PACKET_LOSS_PERC(50));
  ok = opus_encoder_ctl(opus, OPUS_SET_INBAND_FEC(1));
  ok = opus_encoder_ctl(opus, OPUS_SET_DTX(1));
  ok = opus_encoder_ctl(opus, OPUS_SET_PREDICTION_DISABLED(1));
  ok = opus_encoder_ctl(opus, OPUS_SET_VBR(0));
  ok = opus_encoder_ctl(opus, OPUS_SET_BITRATE(60000));//6144));
  ok = opus_encoder_ctl(opus, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_2_5_MS));

  unsigned char encodedOpus[1024] = {0};
  int result = opus_encode(opus, bufIn, 320, encodedOpus, sizeof(encodedOpus));
  
  ///////////
  // CODEC2
  
  struct CODEC2* ctxt = codec2_create(CODEC2_MODE_1200);
  int codec2_1200_samplesPerFrame = codec2_samples_per_frame(ctxt);
  
  int bits = codec2_bits_per_frame(ctxt);
  
  unsigned char encoded[6] = {0};
  codec2_encode(ctxt, encoded, bufIn);

  StartVideoCapture();

  return 0;
}