# define INITGUID

// VP8
# include "./../android/jni/libvpx/vpx/vp8dx.h"
# include "./../android/jni/libvpx/vpx/vp8cx.h"
# include "./../android/jni/libvpx/vpx/vpx_encoder.h"
# include "./../android/jni/libvpx/vpx/vpx_decoder.h"

/*# include "./codecs/libvpx/vp8/common/onyx.h"
# include "./codecs/libvpx/vp8/common/onyxd.h"
# include "./codecs/libvpx/vp8/common/alloccommon.h"
# include "./codecs/libvpx/vp8/decoder/onyxd_int.h"
*/

# include "./../thirdparty/libyuv/include/libyuv.h"

# include <dshow.h>
# include <dvdmedia.h>

# include <ddraw.h>

# pragma comment(lib, "C:\\Program Files\\Windows Kits\\8.0\\Lib\\win8\\um\\x86\\ddraw")
# pragma comment(lib, "strmiids")

// optimized:
// 3/4, 1/2, 3/8, 1/4

// 160 - best minimum usable width for web conferencing.
// 64 - minimum usable (
// 64, 80, 96, 112, 128, 144, _160_, 176, 192, 208, 224, _240_, _320_, _480_, 640

//#15 80x48 1.666
//#28 112x64 1.75
// 40 128x80 1.6
//#60 160x96 1.666
// 84 192x112 1.714
// 112 224x128 1.75
//#135 240x144 1.666
//#240 320x192 1.666
//#510 480x272 1.76
//#920 640x368 1.74
//#2040 960x544 1.76
//#3600 1280x720 1.777
//#8100 1920x1080 1.777

struct VIDEO_STREAM_RESOLUTION
{
  int w;
  int h;
};

const VIDEO_STREAM_RESOLUTION modes[] = {
  {80, 48}, // 0
  {112, 64}, // 1
  {160, 96}, // 2
  {240, 144}, // 3
  {320, 192}, // 4
  {480, 272}, // 5
  {640, 368}, // 6
  {960, 544}, // 7
  {1280, 720}, // 8
  {1920, 1088} // 9
};

#define SHOW_STATS
//# define BYPASS
# define YUV_BILINEAR
# define CODEC_FPS 3
//# define DROP_30_TO_25 99999999// 6

// show first 1 in DROP_30_TO_3 frames
# define DROP_30_TO_3 10 //10
//# define MAXFRAME 5000 //(1480-28)
//# define DROPFRAMEBEFORE 107
//# define DROPFRAMESHOW 7
//# define DROPFRAMEHIDE 4
# define OPUS_BYTES 0//30

//# define HI_Q_PROFILE // 400k

// soft mtu
//# define MTU_PROFILE // 64k (up to 120k)
// hard mtu
//# define MTU_HI_PROFILE

// soft gprs
//# define GPRS_PROFILE // 30k (up to 50)

// hard gprs, needs 3 FPS
# define MTU_LO_PROFILE


//# define USE_THEORA 1 // 16k


# ifdef HI_Q_PROFILE // 640x480 hi-q
# define FRAME_RESOLUTION_NUMBER 6
# define VP8_QP_MAX 56
# define VP8_QP_MIN 2
# define VP8_KBITS_PER_SEC 1//700
# define VP8_CPU_USE -6
//# define MAX_KEYFRAME_DIST 999999
// VBR!!!
# endif

// soft mtu mode, 90k
# ifdef MTU_PROFILE // 240x144 1400 bytes max packet
# define FRAME_RESOLUTION_NUMBER 3
// q 47 c -7 t 1 - 640x480 best q
# define VP8_QP_MAX 55 //56
# define VP8_QP_MIN 0 //56
# define VP8_KBITS_PER_SEC 1 //336//000//10000000 //1 
# define VP8_CPU_USE 0 //-11
# define MAXFRAME (1480-28)
//# define MAX_KEYFRAME_DIST 999999
# endif

// 37 kbit approx, hard limited guaranteed
# ifdef MTU_HI_PROFILE // 160x96 1400 bytes max packet
# define FRAME_RESOLUTION_NUMBER 2
# define VP8_QP_MAX 61 // 58
# define VP8_QP_MIN 0 // <--> with feedback from another side
# define VP8_KBITS_PER_SEC 30//00 //336//000//10000000 //1 
# define VP8_CPU_USE 0 //-11
# define MAXFRAME (1480-28)
//# define MAX_KEYFRAME_DIST 16
# endif

# ifdef GPRS_PROFILE// min bitrate (367 bytes max 20-30kbit/s)
# define FRAME_RESOLUTION_NUMBER 0
# define VP8_QP_MAX 52 //56
# define VP8_QP_MIN 0 //56
# define VP8_KBITS_PER_SEC 1 //336//000//10000000 //1 
# define VP8_CPU_USE 0 //-11
//# define MAX_KEYFRAME_DIST 999999
# endif

// 2267 bit/s 25 fps IPv4 no IPHC
// 6654 bit/s 12,5 fps
# ifdef MTU_LO_PROFILE // 64x32
//# define FRAME_RESOLUTION_NUMBER 0
# define VP8_QP_MAX 52 //53 // 30//63 // 58
# define VP8_QP_MIN 0 // <--> with feedback from another side
# define VP8_KBITS_PER_SEC 7//27//1//00 //336//000//10000000 //1 
# define VP8_CPU_USE 0 //-11
//# define MAXFRAME 94 //(1480-28)

//# define USE_THEORA 1
# endif

# define MAX_KEYFRAME_DIST CODEC_FPS / 3
# define VP8_KEYFRAME_PERCENT MAX_KEYFRAME_DIST * 100


const VIDEO_STREAM_RESOLUTION lowMode = {64, 32};
//const VIDEO_STREAM_RESOLUTION lowMode = {64, 48};

# ifdef MTU_LO_PROFILE
  const VIDEO_STREAM_RESOLUTION* mode = &lowMode;
# else
  const VIDEO_STREAM_RESOLUTION* mode = &modes[FRAME_RESOLUTION_NUMBER];
# endif

int compressedVideoWidth = mode->w;//160;//64; // 128 // 8 // 5
int compressedVideoHeight = mode->h;//compressedVideoWidth * 9 / 16 / 16 * 16; // 96 // 6 // 4

const int viewportW = 1024;
const int viewportH = viewportW * 9 / 16;//compressedVideoHeight / compressedVideoWidth;

# define VP8_THREADS 1 //1

HWND viewport = 0;

int cameraW = 0;
int cameraH = 0;
int camera16_9_H = 0;

uint8* camera16_9_y_plane = 0;
uint8* camera16_9_u_plane = 0;
uint8* camera16_9_v_plane = 0;

uint8* y_plane = 0;
uint8* u_plane = 0;
uint8* v_plane = 0;

void* rgbFrame = 0; // Intermediate surface
void* rgbViewport = 0; // Viewport surface

vpx_codec_ctx_t vp8enc = {0};
vpx_codec_ctx_t vp8dec = {0};
// VP8_COMP* vp8enc = 0;
// VP8D_COMP* vp8dec = 0;

long maxBitrate = 0;
long avgSize = 50;
long minPckt = 10000000;
long maxPckt = 0;

long nframe = 0;
long sec_sz = 0;

int seqFrame = 0;
long totalSent = 0;
long totalSentNoVideo = 0;
long totalFrames = 0;

//IDirectDrawSurface7* viewportSurface = 0;

# include <process.h>

HANDLE wakeEncoder = CreateEvent(0, FALSE, FALSE, 0);
HANDLE wakeDecoder = CreateEvent(0, FALSE, FALSE, 0);

int mediumDuration = 0;

UINT64 pcFrequency = 0;

UINT64 CurrTime()
{
  LARGE_INTEGER pc = {0};
  ::QueryPerformanceCounter(&pc);

  return pc.QuadPart * 1000000 / pcFrequency;
}

uint64_t firstFrameTime = 0;

uint64_t currframeTime = 0;
long currFrameDuration = 0;

HRESULT __stdcall OnNewCameraFrame(void* inst, IMediaSample* smp)
{
  static float fps = 0;

  AM_MEDIA_TYPE* currFmt = 0;
  HRESULT hr1 = smp->GetMediaType(&currFmt);
  if(currFmt)
  {
    VIDEOINFOHEADER* captureFmt = (VIDEOINFOHEADER*)(currFmt->pbFormat);

    fps = float(captureFmt->dwBitRate)
      / float(captureFmt->bmiHeader.biSizeImage * 8);
  }

  BYTE* buf = 0;
  smp->GetPointer(&buf);
  //DWORD len = smp->GetActualDataLength();
  //memcpy(frame, buf, len);

  static uint64_t prevTime = 0;
  currframeTime = CurrTime();

  if(prevTime)
  {
    currFrameDuration = currframeTime - prevTime;
    mediumDuration = (mediumDuration + currFrameDuration)>>1;
  }
  else
  {
    firstFrameTime = currframeTime;
  }

  prevTime = currframeTime;

  //smp->Release();
  HRESULT hr = S_OK;//Receive_(inst, smp);
  //return hr;

# ifdef DROP_30_TO_3
  // show first 1 in 10
  if( ++seqFrame < DROP_30_TO_3 )
  {
    return S_OK;
  }
  else
  {
    seqFrame = 1;
  }

# endif

# ifdef DROP_30_TO_25
  if( ++seqFrame >= DROP_30_TO_25 )
  {
    seqFrame = 0;
    return hr;
  }
# endif

  int offsetY = (cameraH - camera16_9_H)>>1;
  libyuv::YUY2ToI420(
    &buf[(cameraW*offsetY)<<1],
    (cameraW*4)>>1,
    camera16_9_y_plane, cameraW,
    camera16_9_u_plane, cameraW>>1,
    camera16_9_v_plane, cameraW>>1,
    cameraW, camera16_9_H);

  libyuv::I420Scale(
    camera16_9_y_plane, cameraW,
    camera16_9_u_plane, cameraW>>1,
    camera16_9_v_plane, cameraW>>1,
    cameraW, camera16_9_H,
    y_plane, compressedVideoWidth,
    u_plane, compressedVideoWidth>>1,
    v_plane, compressedVideoWidth>>1,
    compressedVideoWidth, compressedVideoHeight, libyuv::kFilterNone);

  SetEvent(wakeEncoder);
  return hr;
}

unsigned char vp8compressedBuf[640000] = {0};
unsigned long vp8compressedSize = sizeof(vp8compressedBuf);

unsigned int __stdcall EncoderThread(void*)
{
  HANDLE thisThread = GetCurrentThread();
  //::SetThreadAffinityMask(thisThread, 0x01);
  ::SetThreadPriority(thisThread, THREAD_PRIORITY_TIME_CRITICAL);
  //::SetThreadPriorityBoost(thisThread, FALSE);

  while(true)
  {
    WaitForSingleObject(wakeEncoder,INFINITE);

    uint64_t currFrameTime = ::currframeTime;
    long currFrameDuration = ::currFrameDuration;
 
    int ret = 0;

# ifndef BYPASS
    vpx_image_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.fmt = VPX_IMG_FMT_I420;
    frame.w = frame.d_w = compressedVideoWidth;
    frame.h = frame.d_h = compressedVideoHeight;
    frame.x_chroma_shift = frame.y_chroma_shift = 1;
    frame.bps = 12;
    frame.img_data_owner = 1;
    frame.img_data = y_plane;
    frame.planes[0] = y_plane;
    frame.planes[1] = u_plane;
    frame.planes[2] = v_plane;
    frame.stride[0] = compressedVideoWidth;
    frame.stride[1] = compressedVideoWidth>>1;
    frame.stride[2] = compressedVideoWidth>>1;
    frame.stride[3] = compressedVideoWidth;

    static int frameIndex = 0;

    int flag = 0;
/*    if (!frameIndex)
    {
      flag = VPX_EFLAG_FORCE_KF;
    }*/

    const vpx_codec_err_t res = vpx_codec_encode(
      &vp8enc, &frame, currFrameTime,
      currFrameDuration ? currFrameDuration/10 : 400000, flag, 8000);
      //VPX_DL_REALTIME);

    vpx_codec_iter_t iter = 0;
    const vpx_codec_cx_pkt_t* pkt = vpx_codec_get_cx_data(&vp8enc, &iter);

    vp8compressedSize = pkt->data.frame.sz;
    memcpy(&vp8compressedBuf, pkt->data.frame.buf, vp8compressedSize);

# endif
 
    SetEvent(wakeDecoder);
  }

  return 0;
}


IDirectDraw7* dd = 0;

unsigned int __stdcall RenderThread(void*)
{
  while(true)
  {
    WaitForSingleObject(wakeDecoder,INFINITE);
 
    ++totalFrames;

# ifdef USE_THEORA
    long pcktsiz = theoraEncoded.bytes + 2 + 6 + 5 + OPUS_BYTES;
# else
    long pcktsiz = vp8compressedSize;// + 234 + 50 + 8 + 67;// + 2 + 6 + 5 + OPUS_BYTES;
# endif

    if (pcktsiz > maxPckt)
    {
      maxPckt = pcktsiz;
    }

    if (pcktsiz < minPckt)
    {
      minPckt = pcktsiz;
    }

    sec_sz += pcktsiz;

    if(nframe++ >=3)
    {
      avgSize = (avgSize+sec_sz)>>1;
      nframe = 0;
      sec_sz = 0;
    }

    totalSent += pcktsiz;
    totalSentNoVideo += pcktsiz - vp8compressedSize;

  # ifndef BYPASS
 
    unsigned char* encPacket = vp8compressedBuf;

# ifdef DROPFRAMEBEFORE
    static int frameNum = 0;
    ++frameNum;
    if(frameNum < DROPFRAMEBEFORE)
    {
      continue;
    }
# endif

# ifdef DROPFRAMESHOW
    {
      static int frameNum = 0;
      if(frameNum > DROPFRAMESHOW)
      {
        frameNum = -DROPFRAMEHIDE;
      }
      if(frameNum++ < 0)
      {
        continue;
      }
    }
# endif

  # ifdef MAXFRAME
    static bool firstTime = true;
    if(pcktsiz>MAXFRAME)
    {
      // drop packet
      packetToDecode = &emptyPacket;
      encPacket = 0;
      vp8compressedSize = 0;
      if(firstTime)
      {
        firstTime = false;
        continue;
      }
    }
  # endif

    vpx_codec_err_t result = vpx_codec_decode(&vp8dec, encPacket, vp8compressedSize,
      0, 0);
    vpx_image_t* img = 0;
    if(VPX_CODEC_OK == result)
    {
      vpx_codec_iter_t iter = 0;
      img = vpx_codec_get_frame(&vp8dec, &iter);
    }

  # endif

  # ifdef BYPASS
      libyuv::I420ToARGB(
        y_plane, compressedVideoWidth,
        u_plane, compressedVideoWidth>>1,
        v_plane, compressedVideoWidth>>1,
        (uint8*)rgbFrame, compressedVideoWidth*4, compressedVideoWidth, compressedVideoHeight);
  # else
    if(img)
    {
      libyuv::I420ToARGB(
        img->planes[0], img->stride[0],
        img->planes[1], img->stride[1],
        img->planes[2], img->stride[2],
        (uint8*)rgbFrame, compressedVideoWidth*4, compressedVideoWidth, compressedVideoHeight);
    }
  # endif

    //DDSURFACEDESC2 ddsd = {0};
    //ddsd.dwSize = sizeof(ddsd);
    //viewportSurface->Lock(0, &ddsd, 0, 0);
  # ifdef YUV_BILINEAR
    libyuv::ARGBScale((uint8*)rgbFrame, compressedVideoWidth * 4,
      compressedVideoWidth, -compressedVideoHeight,
      //(uint8*)ddsd.lpSurface,
      (uint8*)rgbViewport,
      viewportW * 4, viewportW, viewportH,
      libyuv::kFilterBilinear);
  # endif

    //viewportSurface->Unlock(0);

  # ifndef YUV_BILINEAR
    int srcW = compressedVideoWidth;
    int srcH = -compressedVideoHeight;
  # else
    int srcW = viewportW;
    int srcH = viewportH;
  # endif

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    bmi.bmiHeader.biXPelsPerMeter = 3780;
    bmi.bmiHeader.biYPelsPerMeter = 3780;
    bmi.bmiHeader.biClrUsed = 0;
    //bmi.bmiHeader.biWidth = viewportW;
    bmi.bmiHeader.biWidth = srcW;
    //bmi.bmiHeader.biHeight = viewportH;
    bmi.bmiHeader.biHeight = srcH;

  //  dd->WaitForVerticalBlank(0, 0);

    HDC dc = GetDC(viewport);
    StretchDIBits(dc, 0, 0, viewportW, viewportH, 0, 0,
      //viewportW,
      srcW,
      //viewportH,
      abs(srcH),
  # ifdef YUV_BILINEAR
      rgbViewport,
  # else
      rgbFrame,
  # endif
      &bmi, DIB_RGB_COLORS, SRCCOPY);

    float leastTime = (currframeTime - firstFrameTime);
    leastTime /= 1000000.0f;

    static int maxBitrate = 0;
    if (totalFrames % (25*5) == 0)
    {
      maxBitrate = 0;
    }
    int bitrate = 8 * float(totalSent)/leastTime;
      //int(float(totalSent * 8 * 25) / float(totalFrames));
    if(bitrate > maxBitrate)
    {
      maxBitrate = bitrate;
    }

  # ifdef SHOW_STATS
    WCHAR awzMsg[80] = {0};
    //int msgLen = swprintf_s(awzMsg, L"%d %d (%d %d %d)",
    //  totalSent, totalSentNoVideo, minPckt, avgSize/25, maxPckt);
    int msgLen = swprintf_s(awzMsg, L"%d %d (%d %d %d) %f",
      totalSent, maxBitrate, minPckt, avgSize/25, maxPckt,
      1000000.0f/float(mediumDuration));
    TextOutW(dc, 4, 4, awzMsg, msgLen);
  # endif
    ReleaseDC(viewport, dc);
  }
  return 0;
}

void InitVp8Codec()
{
  vpx_codec_iface_t* vp8encIface = vpx_codec_vp8_cx();
  vpx_codec_iface_t* vp8decIface = vpx_codec_vp8_dx();

  vpx_codec_enc_cfg_t cfg = {0};
  vpx_codec_err_t res = vpx_codec_enc_config_default(vp8encIface, &cfg, 0);
  
  cfg.g_w = compressedVideoWidth;
  cfg.g_h = compressedVideoHeight;
  cfg.rc_target_bitrate = VP8_KBITS_PER_SEC;//25 << FRAME_RESOLUTION_NUMBER;

  //cfg.rc_min_quantizer = 2; // WebRTC uses fixed 2
  cfg.rc_min_quantizer = VP8_QP_MIN;//2;//2;//62;
  // WebRTC allows setting via inst.qpMax
  cfg.rc_max_quantizer = VP8_QP_MAX;
  
  cfg.g_timebase.num = 1;
  cfg.g_timebase.den = CODEC_FPS;
  cfg.g_error_resilient = VPX_ERROR_RESILIENT_DEFAULT;// | VPX_ERROR_RESILIENT_PARTITIONS;

  cfg.g_lag_in_frames = 0;

  // Note: WebRTC uses 2 threads for >640x480 when ncores 4up
  // and 3 threads when 1080p and ncores 6up
  cfg.g_threads = VP8_THREADS;

  // WebRTC uses 30 when dropping on.
  cfg.rc_dropframe_thresh = 0;

  // WebRTC allows setting
  cfg.rc_resize_allowed = 0;

  cfg.g_pass = VPX_RC_ONE_PASS;
  cfg.rc_end_usage = VPX_VBR;

  cfg.rc_undershoot_pct = 1000;
  cfg.rc_overshoot_pct = 1000;

  cfg.rc_buf_sz = 666; //1000 / CODEC_FPS;// VP8_KBITS_PER_SEC*1024;
  cfg.rc_buf_initial_sz = 33;//1000 / CODEC_FPS;
  cfg.rc_buf_optimal_sz = 333;//1000 / CODEC_FPS;

  // WebRTC uses SLI & RPSI feedback from decoder and puts
  // this setting to VPX_KF_DISABLED:
  cfg.kf_mode = VPX_KF_AUTO;
  //cfg.kf_mode = VPX_KF_DISABLED;
  // codecSpecific.VP8.keyFrameInterval if not decoder feedback
  cfg.kf_max_dist = MAX_KEYFRAME_DIST;//999999;

  res = vpx_codec_enc_init(&vp8enc, vp8encIface, &cfg, 0);
    //VPX_CODEC_USE_PSNR);//VPX_CODEC_USE_OUTPUT_PARTITION);

  // -12 on ARM, from -6 to -3 usually
  vpx_codec_control(&vp8enc, VP8E_SET_STATIC_THRESHOLD, 0);
  vpx_codec_control(&vp8enc, VP8E_SET_CPUUSED, VP8_CPU_USE);
  vp8e_token_partitions partitions = VP8_ONE_TOKENPARTITION;
  vpx_codec_control(&vp8enc, VP8E_SET_TOKEN_PARTITIONS,
    partitions);
  // On ARM disabled (very slow)
  //vpx_codec_control(&vp8enc, VP8E_SET_NOISE_SENSITIVITY, 1);

  uint32_t targetPct = cfg.rc_buf_optimal_sz * 0.5f * CODEC_FPS / 10;
# ifdef VP8_KEYFRAME_PERCENT
  uint32_t rc_max_intra_target = VP8_KEYFRAME_PERCENT;
# else
  uint32_t rc_max_intra_target = (targetPct < 300) ? 300 : targetPct;
# endif
  vpx_codec_control(&vp8enc, VP8E_SET_MAX_INTRA_BITRATE_PCT,
    rc_max_intra_target);

  // Init decoder

  vpx_codec_dec_cfg_t decCfg = {0};
  decCfg.w = compressedVideoWidth;
  decCfg.h = compressedVideoHeight;
  decCfg.threads = 1;
  res = vpx_codec_dec_init(&vp8dec, vp8decIface, &decCfg,
    VPX_CODEC_USE_POSTPROC | VPX_CODEC_USE_ERROR_CONCEALMENT);
}

void StartVideoCapture()
{
  LARGE_INTEGER pf = {0};
  ::QueryPerformanceFrequency(&pf);
  pcFrequency = pf.QuadPart;

  viewport = CreateWindowW(L"STATIC", L"", WS_POPUP|WS_VISIBLE,
    0, (600-viewportH)>>1,
    viewportW, viewportH, 0, 0, GetModuleHandle(0), 0);

  // src planes for encode
  y_plane = (unsigned char*)malloc(compressedVideoWidth*compressedVideoHeight);
  u_plane = (unsigned char*)malloc((compressedVideoWidth>>1)*(compressedVideoHeight>>1));
  v_plane = (unsigned char*)malloc((compressedVideoWidth>>1)*(compressedVideoHeight>>1));

  InitVp8Codec();

  rgbFrame = malloc(compressedVideoHeight*compressedVideoWidth*4);
  rgbViewport = malloc(viewportW*viewportH*4);

  HRESULT hr = CoInitialize(0);

  hr = DirectDrawCreateEx((GUID*)DDCREATE_HARDWAREONLY, (void**)&dd, IID_IDirectDraw7, 0);
  hr = dd->SetCooperativeLevel(viewport, DDSCL_NORMAL);
/*
  IDirectDrawClipper* clipper = 0;
  hr = dd->CreateClipper(0, &clipper, 0);
  hr = clipper->SetHWnd(0, viewport);
 
  DDSURFACEDESC2 ddsd = {0};
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS;//DDSD_WIDTH;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
  hr = dd->CreateSurface(&ddsd, &viewportSurface, 0);
  hr = viewportSurface->SetClipper(clipper);
*/
/*  HDC dc = GetDC(viewport);
  //hr = dd->GetGDISurface(&viewportSurface);
  hr = dd->GetSurfaceFromDC(dc, &viewportSurface);
  //hr = DDERR_NOTFOUND;

  ReleaseDC(viewport, dc);
*/
//  DDBLTFX fx = {0};
//  fx.dwSize = sizeof(fx);
//  fx.
  //surf->Blt(&rc, surf2, &rc2, 0, &fx);

  HRESULT (__stdcall* Receive_)(void* inst, IMediaSample* smp) = 0;

  IGraphBuilder* graph = 0;
  hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC, IID_IGraphBuilder,
    (void**)&graph);
  IMediaControl* ctrl = 0;
  hr = graph->QueryInterface(IID_IMediaControl, (void**)&ctrl);

  // Select camera
  ICreateDevEnum* devs = 0;
  hr = CoCreateInstance(CLSID_SystemDeviceEnum, 0, CLSCTX_INPROC, IID_ICreateDevEnum,
    (void**)&devs);
  IEnumMoniker* cams = 0;
  hr = devs ? devs->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
    &cams, 0) : hr;
  IMoniker* mon = 0;
  hr = cams->Next(1, &mon, 0); // first camera

  // Create camera
  IBaseFilter* cam = 0;
  hr = mon->BindToObject(0, 0, IID_IBaseFilter, (void**)&cam);
  IEnumPins* pins = 0;
  hr = cam ? cam->EnumPins(&pins) : hr;
  IPin* pin = {0};
  hr = pins ? pins->Next(1, &pin, 0) : hr;

  


  IAMVideoProcAmp* camCtl = 0;
  hr = cam->QueryInterface(IID_IAMVideoProcAmp, (void**)&camCtl);
  if(camCtl)
  {
    long min, max, step, def, flags;
    hr = camCtl->GetRange(VideoProcAmp_Gain, &min, &max, &step, &def, &flags);
    //hr = camCtl->Set(CameraControl_Flags_Auto, 0, 0);
    //hr = camCtl->Set(CameraControl_Flags_Manual, 1, CameraControl_Flags_Manual);
    hr = camCtl->Set(VideoProcAmp_Gain, 1, VideoProcAmp_Flags_Manual);
  }

  /*
  IAMStreamConfig* cfg = 0;
  hr = pin->QueryInterface(IID_IAMStreamConfig, (void**)&cfg);
  int sz, max_res = 0;
  hr = cfg->GetNumberOfCapabilities(&max_res, &sz);
  VIDEO_STREAM_CONFIG_CAPS cap[2] = {0};
  AM_MEDIA_TYPE* fmt = 0;
  hr = cfg->GetStreamCaps(max_res-1, &fmt, (BYTE*)cap);

  AM_MEDIA_TYPE* currFmt = 0;
  cfg->GetFormat(&currFmt);
  // now cap contains max width and height.
  // 1 second = 10 000 000
  VIDEOINFOHEADER* captureFmt = (VIDEOINFOHEADER*)(currFmt->pbFormat);
  captureFmt->AvgTimePerFrame *= 4;
  captureFmt->dwBitRate /= 4;
  hr = cfg->SetFormat(0);
  hr = cfg->SetFormat(currFmt);*/

  IAMStreamConfig* cfg = 0;
  hr = pin->QueryInterface(IID_IAMStreamConfig, (void**)&cfg);
  AM_MEDIA_TYPE* currFmt = 0;
  cfg->GetFormat(&currFmt);
  // 1 second = 10 000 000
  VIDEOINFOHEADER* captureFmt = (VIDEOINFOHEADER*)(currFmt->pbFormat);

  float fps = float(captureFmt->dwBitRate)
    / float(captureFmt->bmiHeader.biSizeImage * 8);// bitrate for 640x480 - 147456000
/*
  captureFmt->bmiHeader.biWidth = 160;
  captureFmt->bmiHeader.biHeight = 120;
  //captureFmt->AvgTimePerFrame <<= 1;
  captureFmt->bmiHeader.biSizeImage
    = captureFmt->bmiHeader.biWidth * captureFmt->bmiHeader.biHeight
    * captureFmt->bmiHeader.biBitCount / 8;
  captureFmt->dwBitRate
    = (10000000/captureFmt->AvgTimePerFrame) * captureFmt->bmiHeader.biSizeImage * 8;
  cfg->SetFormat(currFmt);
*/
  fps = float(captureFmt->dwBitRate)
    / float(captureFmt->bmiHeader.biSizeImage * 8);

  cameraW = captureFmt->bmiHeader.biWidth;
  cameraH = captureFmt->bmiHeader.biHeight;

  camera16_9_H = cameraW * 9 / 16;

  camera16_9_y_plane = (unsigned char*)malloc(cameraW*camera16_9_H);
  camera16_9_u_plane = (unsigned char*)malloc((cameraW*camera16_9_H)>>1);
  camera16_9_v_plane = (unsigned char*)malloc((cameraW*camera16_9_H)>>1);

  // Place camera
  hr = graph->AddFilter(cam, L"Capture Source");
  // Connect camera (preview created automatically)
  hr = graph->Render(pin);

  // Find renderer
  IEnumFilters* fil = 0;
  hr = graph->EnumFilters(&fil);
  IBaseFilter* renderer = 0;
  hr = fil->Next(1, &renderer, 0);

  // Enum renderer pins
  hr = renderer->EnumPins(&pins);
  IPin* pin2 = 0;
  hr = pins->Next(1, &pin2, 0);

  IMemInputPin* mem = 0;
  hr = pin2->QueryInterface(IID_IMemInputPin, (void**)&mem);

  if(!Receive_)
  {
    INT_PTR* p = 6 + *(INT_PTR**)mem;
    DWORD no = 0;
    VirtualProtect(&Receive_, 4, PAGE_EXECUTE_READWRITE, &no);
    *((INT_PTR*)&Receive_) = *p;
    VirtualProtect(p, 4, PAGE_EXECUTE_READWRITE, &no);
    *p = (INT_PTR)OnNewCameraFrame;
  }

  //IVideoWindow* window = 0;
  //hr = renderer->QueryInterface(IID_IVideoWindow, (void**)&window);
  //window->put_Owner((OAHWND)viewport);
  //window->put_AutoShow(OAFALSE);
  renderer->Stop();

  cam->SetSyncSource(0);
  hr = cam->Run(0);
  if(S_OK == hr)
  {
    _beginthreadex(0, 0, EncoderThread, 0, 0, 0);
    _beginthreadex(0, 0, RenderThread, 0, 0, 0);

    // Run "scene"
    hr = ctrl->Run();

    MSG msg = {0};
    while(GetMessage(&msg, 0, 0, 0))
    {
      DispatchMessage(&msg);
      if(msg.message == WM_SYSCOMMAND)
      {
        ExitProcess(0);
      }
    }
  }
}