#include <jni.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <SLES/OpenSLES_AndroidConfiguration.h>
//#include <SLES/OpenSLES_AndroidMetadata.h>

#include <sys/time.h>
#include <stdio.h>

#include "./../../thirdparty/libyuv/include/libyuv.h"

# include "./libvpx/vpx/vp8dx.h"
# include "./libvpx/vpx/vp8cx.h"
# include "./libvpx/vpx/vpx_encoder.h"
# include "./libvpx/vpx/vpx_decoder.h"

#include "./../../thirdparty/codec2/src/codec2.h"

static JavaVM* g_vm = 0;

extern "C" jint JNIEXPORT JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
  // Only called once.
//  CHECK(!g_vm, "OnLoad called more than once");
  g_vm = vm;
  return JNI_VERSION_1_4;
}

vpx_codec_ctx_t vp8enc = {0};
vpx_codec_ctx_t vp8dec = {0};

# define VP8_CPU_USE -16

void InitVp8Codec()
{
  vpx_codec_iface_t* vp8encIface = vpx_codec_vp8_cx();
  vpx_codec_iface_t* vp8decIface = vpx_codec_vp8_dx();

  vpx_codec_enc_cfg_t cfg = {0};
  vpx_codec_err_t res = vpx_codec_enc_config_default(vp8encIface, &cfg, 0);

  cfg.g_w = 320;
  cfg.g_h = 240;
  cfg.rc_target_bitrate = 10000;//3400;//25 << FRAME_RESOLUTION_NUMBER;

  //cfg.rc_min_quantizer = 2; // WebRTC uses fixed 2
  cfg.rc_min_quantizer = 63;//2;//2;//62;
  // WebRTC allows setting via inst.qpMax
  cfg.rc_max_quantizer = 63;

  cfg.g_timebase.num = 1;
  cfg.g_timebase.den = 3;
  cfg.g_error_resilient = VPX_ERROR_RESILIENT_DEFAULT;// | VPX_ERROR_RESILIENT_PARTITIONS;

  cfg.g_lag_in_frames = 0;

  // Note: WebRTC uses 2 threads for >640x480 when ncores 4up
  // and 3 threads when 1080p and ncores 6up
  cfg.g_threads = 1;

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
  cfg.kf_max_dist = 30;//999999;

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

  uint32_t targetPct = cfg.rc_buf_optimal_sz * 0.5f * 30 / 10;
# ifdef VP8_KEYFRAME_PERCENT
  uint32_t rc_max_intra_target = VP8_KEYFRAME_PERCENT;
# else
  uint32_t rc_max_intra_target = (targetPct < 300) ? 300 : targetPct;
# endif
  vpx_codec_control(&vp8enc, VP8E_SET_MAX_INTRA_BITRATE_PCT,
    rc_max_intra_target);

  // Init decoder

  vpx_codec_dec_cfg_t decCfg = {0};
  decCfg.w = 320;
  decCfg.h = 240;
  decCfg.threads = 1;
  res = vpx_codec_dec_init(&vp8dec, vp8decIface, &decCfg,
    VPX_CODEC_USE_POSTPROC | VPX_CODEC_USE_ERROR_CONCEALMENT);
}

extern void StartMyCapture(JavaVM* vm);

static bool singleIn = false;

char extText[64] = {0};

JNIEnv* g_env = 0;
jobject g_obj = 0;
void SetLabel(const char* text)
{
	JNIEnv* env = g_env;
	jobject obj = g_obj;
	jstring jstr = env->NewStringUTF(text);
	jclass clazz = env->GetObjectClass(obj);
	jmethodID messageMe = env->GetMethodID(clazz, "messageMe", "(Ljava/lang/String;)Ljava/lang/String;");
	jobject result = env->CallObjectMethod(obj, messageMe, jstr);
}

double microsecs()
{
  timeval _t = {0};
  gettimeofday(&_t, 0);
  return (double)_t.tv_sec + (double)_t.tv_usec/(1000*1000);
}

uint8* camera_y_plane = 0;
uint8* camera_u_plane = 0;
uint8* camera_v_plane = 0;

char vp8compressedBuf[40000];

void OnCameraFrame(void* data)
{
	double currTime = microsecs();
	static double prevTime = currTime;

	uint64_t currFrameTime = currTime*1000;
	long currFrameDuration = (currTime - prevTime)*1000;

	//static int frameN = 0;
	sprintf(extText, "%f s\n", currTime - prevTime);
	prevTime = currTime;

	libyuv::YUY2ToI420(
	    (uint8*)data,
	    (320*4)>>1,
	    camera_y_plane, 320,
	    camera_u_plane, 320>>1,
	    camera_v_plane, 320>>1,
	    320, 240);

	vpx_image_t frame;
	memset(&frame, 0, sizeof(frame));
	frame.fmt = VPX_IMG_FMT_I420;
	frame.w = frame.d_w = 320;
	frame.h = frame.d_h = 240;
	frame.x_chroma_shift = frame.y_chroma_shift = 1;
	frame.bps = 12;
	frame.img_data_owner = 1;
	frame.img_data = camera_y_plane;
	frame.planes[0] = camera_y_plane;
	frame.planes[1] = camera_u_plane;
	frame.planes[2] = camera_v_plane;
	frame.stride[0] = 320;
	frame.stride[1] = 320>>1;
	frame.stride[2] = 320>>1;
	frame.stride[3] = 320;

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

	int vp8compressedSize = pkt->data.frame.sz;
	memcpy(&vp8compressedBuf, pkt->data.frame.buf, vp8compressedSize);
}

extern "C" {

jstring Java_com_aeonsoftech_guruchat_LoginActivity_getJniString(JNIEnv* env, jobject obj)
{
	g_env = env;
	g_obj = obj;

	SetLabel(extText);

	if (singleIn)
		return env->NewStringUTF("abc");

	camera_y_plane = (unsigned char*)malloc(320*240);
	camera_u_plane = (unsigned char*)malloc((320*240)>>1);
	camera_v_plane = (unsigned char*)malloc((320*240)>>1);

	singleIn = true;

	StartMyCapture(g_vm);

    InitVp8Codec();

	short bufIn[320];

	struct CODEC2* ctxt = codec2_create(CODEC2_MODE_1200);
	int codec2_1200_samplesPerFrame = codec2_samples_per_frame(ctxt);

	int bits = codec2_bits_per_frame(ctxt);

	unsigned char encoded[6] = {0};
	codec2_encode(ctxt, encoded, bufIn);

	vpx_codec_iface_t* vp8encIface = vpx_codec_vp8_cx();

//	SetLabel("This comes from jni.");

    //const char* str = env->GetStringUTFChars((jstring) result, 0); // should be released but what a heck, it's a tutorial :)
    //printf("%s\n", str);

    return env->NewStringUTF("abc");//str);
}

}
