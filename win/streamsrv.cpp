# include <WinSock2.h>

extern "C"
{
# include "./../thirdparty/codec2/src/codec2.h"
# include "./../thirdparty/opus/include/opus.h"
# include "./../thirdparty/oslec/spandsp-0.0.3/src/msvc/inttypes.h"
}

# include "audiostream.h"
# include <malloc.h>
# include <intrin.h>


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
  serverAddr.sin_addr.S_un.S_addr = inet_addr("212.79.111.155");
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

  StunRequest* response = (StunRequest*)recvBuffer;

}


int WINAPI wWinMain(HINSTANCE, HINSTANCE, wchar_t*, int)
//int main()
{
  // Max. bitrate: 512000 for audio, 20 000 000 for video.
  // ~30 MBit/s

  //DetermineExternalAddress();

  CaptureAudio();

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