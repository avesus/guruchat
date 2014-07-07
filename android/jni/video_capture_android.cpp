/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

# include <jni.h>

//#define assert(val)
//#define NULL 0
# include <inttypes.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

//#include "webrtc/modules/utility/interface/helpers_android.h"
//#include "device_info_android.h"
//#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"
//#include "webrtc/system_wrappers/interface/logcat_trace_context.h"
//#include "webrtc/system_wrappers/interface/ref_count.h"
//#include "webrtc/system_wrappers/interface/trace.h"

static JavaVM* g_jvm = NULL;
static jclass g_java_capturer_class = NULL;  // VideoCaptureAndroid.class.

extern void OnCameraFrame(void* data);

class VideoCaptureCapability
{
};

class VideoCaptureRotation
{
};

class AttachThreadScoped {
 public:
  explicit AttachThreadScoped(JavaVM* jvm);
  ~AttachThreadScoped();
  JNIEnv* env();

 private:
  bool attached_;
  JavaVM* jvm_;
  JNIEnv* env_;
};

AttachThreadScoped::AttachThreadScoped(JavaVM* jvm)
    : attached_(false), jvm_(jvm), env_(NULL) {
  jint ret_val = jvm->GetEnv(reinterpret_cast<void**>(&env_), JNI_VERSION_1_4);
  if (ret_val == JNI_EDETACHED) {
    // Attach the thread to the Java VM.
    ret_val = jvm_->AttachCurrentThread(&env_, NULL);
    attached_ = ret_val == JNI_OK;
    assert(attached_);
  }
}

AttachThreadScoped::~AttachThreadScoped() {
  if (attached_ && (jvm_->DetachCurrentThread() < 0)) {
    assert(false);
  }
}

JNIEnv* AttachThreadScoped::env() { return env_; }

//D:\dev\other\webrtc\webrtc\system_wrappers\interface\critical_section_wrapper.h
//#include "./../../../other/webrtc/webrtc/common_types.h"
#include "./../../../other/webrtc/webrtc/system_wrappers/interface/thread_annotations.h"

class LOCKABLE CriticalSectionWrapper {
 public:
  // Factory method, constructor disabled
  static CriticalSectionWrapper* CreateCriticalSection();

  virtual ~CriticalSectionWrapper() {}

  // Tries to grab lock, beginning of a critical section. Will wait for the
  // lock to become available if the grab failed.
  virtual void Enter() EXCLUSIVE_LOCK_FUNCTION() = 0;

  // Returns a grabbed lock, end of critical section.
  virtual void Leave() UNLOCK_FUNCTION() = 0;
};

class CriticalSectionPosix : public CriticalSectionWrapper {
 public:
  CriticalSectionPosix();

  virtual ~CriticalSectionPosix();

  virtual void Enter();
  virtual void Leave();

 private:
  pthread_mutex_t mutex_;
  friend class ConditionVariablePosix;
};

CriticalSectionPosix::CriticalSectionPosix() {
  pthread_mutexattr_t attr;
  (void) pthread_mutexattr_init(&attr);
  (void) pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  (void) pthread_mutex_init(&mutex_, &attr);
}

CriticalSectionPosix::~CriticalSectionPosix() {
  (void) pthread_mutex_destroy(&mutex_);
}

void
CriticalSectionPosix::Enter() {
  (void) pthread_mutex_lock(&mutex_);
}

void
CriticalSectionPosix::Leave() {
  (void) pthread_mutex_unlock(&mutex_);
}

CriticalSectionWrapper* CriticalSectionWrapper::CreateCriticalSection() {
#ifdef _WIN32
  return new CriticalSectionWindows();
#else
  return new CriticalSectionPosix();
#endif
}

// RAII extension of the critical section. Prevents Enter/Leave mismatches and
// provides more compact critical section syntax.
class SCOPED_LOCKABLE CriticalSectionScoped {
 public:
  explicit CriticalSectionScoped(CriticalSectionWrapper* critsec)
      EXCLUSIVE_LOCK_FUNCTION(critsec)
      : ptr_crit_sec_(critsec) {
    ptr_crit_sec_->Enter();
  }

  ~CriticalSectionScoped() UNLOCK_FUNCTION() { ptr_crit_sec_->Leave(); }

 private:
  CriticalSectionWrapper* ptr_crit_sec_;
};

class VideoCaptureAndroid {
 public:
  VideoCaptureAndroid(const int32_t id):
  //_deviceInfo(id),
  _jCapturer(NULL),
  _captureStarted(false) {
}
  virtual int32_t Init(const int32_t id, const char* deviceUniqueIdUTF8);

  virtual int32_t StartCapture(const VideoCaptureCapability& capability);
  virtual int32_t StopCapture();
  virtual bool CaptureStarted();
  virtual int32_t CaptureSettings(VideoCaptureCapability& settings);
  virtual int32_t SetCaptureRotation(VideoCaptureRotation rotation);

  int32_t OnIncomingFrame(uint8_t* videoFrame,
                                               int32_t videoFrameLength,
                                               int64_t captureTime = 0)
  {
	  OnCameraFrame(videoFrame);

	  return 0;
  }

  virtual ~VideoCaptureAndroid(){
	  // Ensure Java camera is released even if our caller didn't explicitly Stop.
	  if (_captureStarted)
	    StopCapture();
	  AttachThreadScoped ats(g_jvm);
	  ats.env()->DeleteGlobalRef(_jCapturer);
	}

private:
  //DeviceInfoAndroid _deviceInfo;
  jobject _jCapturer; // Global ref to Java VideoCaptureAndroid object.
  VideoCaptureCapability _captureCapability;
  bool _captureStarted;
};

extern void OnCameraFrame(void* data);

// Called by Java when the camera has a new frame to deliver.
void JNICALL ProvideCameraFrame(
    JNIEnv* env,
    jobject,
    jbyteArray javaCameraFrame,
    jint length,
    jlong context) {
  VideoCaptureAndroid* captureModule =
      reinterpret_cast<VideoCaptureAndroid*>(
          context);
  jbyte* cameraFrame = env->GetByteArrayElements(javaCameraFrame, NULL);
  captureModule->OnIncomingFrame(
      reinterpret_cast<uint8_t*>(cameraFrame), length, 0);
  env->ReleaseByteArrayElements(javaCameraFrame, cameraFrame, JNI_ABORT);
}

int32_t SetCaptureAndroidVM(JavaVM* javaVM) {
  assert(!g_jvm);
  g_jvm = javaVM;
  AttachThreadScoped ats(g_jvm);

 // videocapturemodule::DeviceInfoAndroid::Initialize(ats.env());

  jclass j_capture_class =
      ats.env()->FindClass("com/aeonsoftech/guruchat/VideoCaptureAndroid");
  assert(j_capture_class);
  g_java_capturer_class =
      reinterpret_cast<jclass>(ats.env()->NewGlobalRef(j_capture_class));
  assert(g_java_capturer_class);

  JNINativeMethod native_method = {
    "ProvideCameraFrame", "([BIJ)V",
    reinterpret_cast<void*>(&ProvideCameraFrame)
  };
  if (ats.env()->RegisterNatives(g_java_capturer_class, &native_method, 1) != 0)
    assert(false);

  return 0;
}

static VideoCaptureAndroid cap(0);

void StartMyCapture(JavaVM* javaVM)
{
	SetCaptureAndroidVM(javaVM);

	cap.Init(0, "abc");
	VideoCaptureCapability cp;
	cap.StartCapture(cp);
}

int32_t VideoCaptureAndroid::Init(const int32_t id,
                                  const char* deviceUniqueIdUTF8) {
  const int nameLength = strlen(deviceUniqueIdUTF8);

  // Store the device name
  //_deviceUniqueId = new char[nameLength + 1];
  //memcpy(_deviceUniqueId, deviceUniqueIdUTF8, nameLength + 1);

  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();

  jmethodID ctor = env->GetMethodID(g_java_capturer_class, "<init>", "(IJ)V");
  assert(ctor);
  jlong j_this = reinterpret_cast<intptr_t>(this);
  size_t camera_id = 0;
  //if (!_deviceInfo.FindCameraIndex(deviceUniqueIdUTF8, &camera_id))
  //  return -1;
  _jCapturer = env->NewGlobalRef(
      env->NewObject(g_java_capturer_class, ctor, camera_id, j_this));
  assert(_jCapturer);
  return 0;
}

CriticalSectionWrapper& _apiCs = *CriticalSectionWrapper::CreateCriticalSection();

int32_t VideoCaptureAndroid::StartCapture(
    const VideoCaptureCapability& capability) {
  CriticalSectionScoped cs(&_apiCs);
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();

/*  if (_deviceInfo.GetBestMatchedCapability(
          _deviceUniqueId, capability, _captureCapability) < 0) {
    WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, -1,
                 "%s: GetBestMatchedCapability failed: %dx%d",
                 __FUNCTION__, capability.width, capability.height);
    return -1;
  }
*/
  //_captureDelay = 0;//_captureCapability.expectedCaptureDelay;

  jmethodID j_start =
      env->GetMethodID(g_java_capturer_class, "startCapture", "(IIII)Z");
  assert(j_start);
  int min_mfps = 1;
  int max_mfps = 30;
  bool started = env->CallBooleanMethod(_jCapturer, j_start,
                                        320,
                                        240,
                                        min_mfps, max_mfps);
  if (started) {
    //_requestedCapability = capability;
    _captureStarted = true;
  }
  return started ? 0 : -1;
}

int32_t VideoCaptureAndroid::StopCapture() {
  CriticalSectionScoped cs(&_apiCs);
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();

  //memset(&_requestedCapability, 0, sizeof(_requestedCapability));
  memset(&_captureCapability, 0, sizeof(_captureCapability));
  _captureStarted = false;

  jmethodID j_stop =
      env->GetMethodID(g_java_capturer_class, "stopCapture", "()Z");
  return env->CallBooleanMethod(_jCapturer, j_stop) ? 0 : -1;
}

bool VideoCaptureAndroid::CaptureStarted() {
  CriticalSectionScoped cs(&_apiCs);
  return _captureStarted;
}

int32_t VideoCaptureAndroid::CaptureSettings(
    VideoCaptureCapability& settings) {
  CriticalSectionScoped cs(&_apiCs);
  //settings = _requestedCapability;
  return 0;
}

int32_t VideoCaptureAndroid::SetCaptureRotation(
    VideoCaptureRotation rotation) {
  CriticalSectionScoped cs(&_apiCs);
/*  if (VideoCaptureImpl::SetCaptureRotation(rotation) != 0)
    return 0;
*/
  AttachThreadScoped ats(g_jvm);
  JNIEnv* env = ats.env();

  jmethodID j_spr =
      env->GetMethodID(g_java_capturer_class, "setPreviewRotation", "(I)V");
  assert(j_spr);
  int rotation_degrees;
/*  if (RotationInDegrees(rotation, &rotation_degrees) != 0) {
    assert(false);
  }*/
  env->CallVoidMethod(_jCapturer, j_spr, rotation_degrees);
  return 0;
}
