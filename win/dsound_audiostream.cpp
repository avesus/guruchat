# include "audiostream.h"
# define INITGUID
# include <dsound.h>
# pragma comment(lib, "dsound")

# include <fvec.h>
# include <ivec.h>

//# include "inttypes.h"

# include "./../thirdparty/oslec/spandsp-0.0.3/src/msvc/inttypes.h"
// common defs
# include "./../thirdparty/oslec/spandsp-0.0.3/src/spandsp/telephony.h"
extern "C"{
# include "./../thirdparty/oslec/spandsp-0.0.3/src/spandsp/echo.h"
}

# include "./../thirdparty/oslec/spandsp-0.0.3/src/spandsp/noise.h"
# include "./../thirdparty/oslec/spandsp-0.0.3/src/spandsp/power_meter.h"
// jitter buffer
# include "./../thirdparty/oslec/spandsp-0.0.3/src/spandsp/playout.h"
// Packet loss concealment fill-in
# include "./../thirdparty/oslec/spandsp-0.0.3/src/spandsp/plc.h"

// VAD
# define USE_ALLOCA
# include <opus.h>
# include "./../thirdparty/opus/celt/ecintrin.h"
# include "./../thirdparty/opus/silk/structs.h"
extern "C"{
# include "./../thirdparty/opus/silk/main.h"
}

# include <time.h>

GUID audioCaptureDevs[16] = {0};

BOOL CALLBACK CaptureAudioDevices(GUID* dev,
  const WCHAR* name1, const WCHAR* name2, void* ctxt)
{
  static int n = 0;
  if(dev)
  {
    audioCaptureDevs[n++] = *dev;
  }
  return TRUE;
}

void CaptureAudio()
{
  echo_can_state_t* echoCanceler = echo_can_create(4096, 0);
  int16_t result = echo_can_update(echoCanceler, 0, 0);

  noise_state_t noiseGen = {0};
  time_t noiseSeed = time(0);
  float dbm0Level = DBM0_MAX_POWER;
  int quality = 20; // 4..20
  noise_init_dbm0(&noiseGen, (int)noiseSeed,
    dbm0Level, NOISE_CLASS_AWGN, quality);

  power_meter_t powerMeter = {0};
  power_meter_init(&powerMeter, 6);

  opus_int16 audioData[32768] = {0};
  for(int i = 0; i < 32768; ++i)
  {
    int16_t sample = noise(&noiseGen);
    audioData[i] = sample;
    power_meter_update(&powerMeter, sample);
  }

  float dbm0Metered = power_meter_dbm0(&powerMeter);

  silk_encoder_state silkStub = {0};
  // 320 max
  silkStub.frame_length = 320;
  // Internal sampling frequency
  silkStub.fs_kHz = 8;

  silk_VAD_Init(&silkStub.sVAD);

  silk_VAD_GetSA_Q8(&silkStub, audioData);

  silkStub.input_tilt_Q15;
  silkStub.input_quality_bands_Q15;
  silkStub.speech_activity_Q8;

  /*
  if( psEnc->sCmn.speech_activity_Q8 < SILK_FIX_CONST( SPEECH_ACTIVITY_DTX_THRES, 8 ) ) {
        psEnc->sCmn.indices.signalType = TYPE_NO_VOICE_ACTIVITY;
        psEnc->sCmn.noSpeechCounter++;
        if( psEnc->sCmn.noSpeechCounter < NB_SPEECH_FRAMES_BEFORE_DTX ) {
            psEnc->sCmn.inDTX = 0;
        } else if( psEnc->sCmn.noSpeechCounter > MAX_CONSECUTIVE_DTX + NB_SPEECH_FRAMES_BEFORE_DTX ) {
            psEnc->sCmn.noSpeechCounter = NB_SPEECH_FRAMES_BEFORE_DTX;
            psEnc->sCmn.inDTX           = 0;
        }
        psEnc->sCmn.VAD_flags[ psEnc->sCmn.nFramesEncoded ] = 0;
    } else {
        psEnc->sCmn.noSpeechCounter    = 0;
        psEnc->sCmn.inDTX              = 0;
        psEnc->sCmn.indices.signalType = TYPE_UNVOICED;
        psEnc->sCmn.VAD_flags[ psEnc->sCmn.nFramesEncoded ] = 1;
    }
  */


  HRESULT hr = ::DirectSoundCaptureEnumerateW(CaptureAudioDevices, 0);
  IDirectSoundCapture* dsc = 0;
  hr = ::DirectSoundCaptureCreate(&audioCaptureDevs[0], &dsc, 0);

  WAVEFORMATEX wf = {0};
  wf.cbSize = sizeof(wf);
  wf.nAvgBytesPerSec = 2 * 16000;
  wf.nBlockAlign = 2;
  wf.nChannels = 1;
  wf.nSamplesPerSec = 16000;
  wf.wBitsPerSample = 16;
  wf.wFormatTag = WAVE_FORMAT_PCM;

  int audioFrameLength = 5 * wf.nAvgBytesPerSec / 1000;
  int audioBufferFrames = 1;

  DSCBUFFERDESC dscbd = {0};
  dscbd.dwSize = sizeof(dscbd);
  dscbd.dwBufferBytes = audioFrameLength * audioBufferFrames;
  dscbd.lpwfxFormat = &wf;
  IDirectSoundCaptureBuffer* dscb = 0;
  hr = dsc->CreateCaptureBuffer(&dscbd, &dscb, 0);

  IDirectSoundNotify* pDSBNotify = 0;
	hr = dscb->QueryInterface(IID_IDirectSoundNotify, (void**)&pDSBNotify);

	//Set Direct Sound Buffer Notify Position
/*  DSBPOSITIONNOTIFY pPosNotify[2] = {0};
	pPosNotify[0].dwOffset = m_WFE.nAvgBytesPerSec/2 - 1;
	pPosNotify[1].dwOffset = 3*m_WFE.nAvgBytesPerSec/2 - 1;		
	pPosNotify[0].hEventNotify = m_pHEvent[0];
	pPosNotify[1].hEventNotify = m_pHEvent[1];

	hr = pDSBNotify->SetNotificationPositions(2, pPosNotify);
*/
  hr = dscb->Start(0);
  void* ptr = 0;
  DWORD nBytes = 0;

  hr = dscb->Lock(0, 0, &ptr, &nBytes, 0, 0, 0);

  dscb->Stop();
  dscb->Release();

  dsc->Release();
}