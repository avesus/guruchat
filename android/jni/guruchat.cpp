#include <jni.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <SLES/OpenSLES_AndroidConfiguration.h>
//#include <SLES/OpenSLES_AndroidMetadata.h>

#include "./../../thirdparty/libyuv/include/libyuv.h"

# include "./libvpx/vpx/vp8dx.h"
# include "./libvpx/vpx/vp8cx.h"
# include "./libvpx/vpx/vpx_encoder.h"
# include "./libvpx/vpx/vpx_decoder.h"

#include "./../../thirdparty/codec2/src/codec2.h"

extern "C" {

jstring Java_com_aeonsoftech_guruchat_LoginActivity_getJniString( JNIEnv* env, jobject obj)
{

	short bufIn[320];

	struct CODEC2* ctxt = codec2_create(CODEC2_MODE_1200);
	int codec2_1200_samplesPerFrame = codec2_samples_per_frame(ctxt);

	int bits = codec2_bits_per_frame(ctxt);

	unsigned char encoded[6] = {0};
	codec2_encode(ctxt, encoded, bufIn);

	vpx_codec_iface_t* vp8encIface = vpx_codec_vp8_cx();

	libyuv::YUY2ToI420(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    jstring jstr = env->NewStringUTF("This comes from jni.");
    jclass clazz = env->GetObjectClass(obj);
    jmethodID messageMe = env->GetMethodID(clazz, "messageMe", "(Ljava/lang/String;)Ljava/lang/String;");
    jobject result = env->CallObjectMethod(obj, messageMe, jstr);

    const char* str = env->GetStringUTFChars((jstring) result, 0); // should be released but what a heck, it's a tutorial :)
    //printf("%s\n", str);

    return env->NewStringUTF(str);
}

}
