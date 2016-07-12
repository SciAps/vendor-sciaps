#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define LOG_TAG "Screencast"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/IMemory.h>
#include <utils/Thread.h>
#include <utils/Timers.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <ui/DisplayInfo.h>

#include <gui/Surface.h>
#include <gui/SurfaceTextureClient.h>

#include <media/openmax/OMX_IVCommon.h>

#include <media/ICrypto.h>
#include <media/stagefright/MediaCodecList.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/ColorConverter.h>

#define DEFAULT_TIMEOUT 1000

using namespace android;

static uint32_t DEFAULT_DISPLAY_ID = ISurfaceComposer::eDisplayIdMain;

static uint32_t gVideoWidth = 0;            // default width+height
static uint32_t gVideoHeight = 0;
static uint32_t gBitRate = 4000000;         // 4Mbps
static uint32_t gTimeLimitSec = 180;      // 3 minutes
static float gVideoFPS = 12.0f;

status_t prepareEncoder(sp<MediaCodec>& oCodec) {
  status_t err;

  sp<AMessage> format = new AMessage;
  format->setInt32("width", gVideoWidth);
  format->setInt32("height", gVideoHeight);
  format->setString("mime", "video/avc");
  format->setInt32("color-format", OMX_TI_COLOR_FormatYUV420PackedSemiPlanar);
  format->setInt32("bitrate", gBitRate);
  format->setFloat("frame-rate", gVideoFPS);
  format->setInt32("i-frame-interval", 10);

  sp<ALooper> looper = new ALooper;
  looper->setName("screencast_looper");
  looper->start();

  sp<MediaCodec> codec = MediaCodec::CreateByType(looper, "video/avc", true);
  if (codec == NULL) {
      printf("ERROR: unable to create video/avc codec instance\n");
      return UNKNOWN_ERROR;
  }

  err = codec->configure(format, NULL, NULL, MediaCodec::CONFIGURE_FLAG_ENCODE);
  if (err != NO_ERROR) {
      codec->release();
      codec.clear();
      printf("ERROR: unable to configure codec (err=%d)\n", err);
      return err;
  }

  oCodec = codec;

  return NO_ERROR;
}

static status_t argb8888ToTIYUV420PackedSemiPlanar(const void* srcPtr, void* destPtr, size_t w, size_t h,
   size_t* destSize = 0) {

     const size_t frameSize = w * h;

     uint32_t* argb = (uint32_t*)srcPtr;
     uint8_t* yuv420sp = (uint8_t*)destPtr;
     size_t x, y;
     int a, R, G, B, Y, U, V;
     size_t index = 0;
     size_t yIndex = 0;
     size_t uvIndex = frameSize;

     if(destSize != 0) {
       *destSize = w*h*3/2;
     }

     for(y=0;y<h;y++) {
       for(x=0;x<w;x++) {
         a = (argb[index] & 0xff000000) >> 24; // a is not used
         R = (argb[index] & 0xff0000) >> 16;
         G = (argb[index] & 0xff00) >> 8;
         B = (argb[index] & 0xff) >> 0;

         // well known RGB to YUV algorithm
         Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
         U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
         V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

         // NV21 has a plane of Y and interleaved planes of VU each sampled by a factor of 2
         //    meaning for every 4 Y pixels there are 1 V and 1 U.  Note the sampling is every other
         //    pixel AND every other scanline.
         yuv420sp[yIndex++] = (uint8_t) ((Y < 0) ? 0 : ((Y > 255) ? 255 : Y));
         if (y % 2 == 0 && index % 2 == 0) {
             yuv420sp[uvIndex++] = (uint8_t)((V<0) ? 0 : ((V > 255) ? 255 : V));
             yuv420sp[uvIndex++] = (uint8_t)((U<0) ? 0 : ((U > 255) ? 255 : U));
         }
         index++;

       }
     }

     return OK;

}

static status_t screenShotToBuffer(const sp<IBinder>& display, sp<ABuffer>& buffer, size_t dstSize) {

  size_t w, h;
  status_t err;
  //size_t size = 0;
  ScreenshotClient screenshot;

  err = screenshot.update(display);
  if (err == NO_ERROR) {
      const void* srcPtr = screenshot.getPixels();
      void* dstPtr = buffer->data();

      w = screenshot.getWidth();
      h = screenshot.getHeight();
      //PixelFormat f = screenshot.getFormat();
      //size = screenshot.getSize();

      err = argb8888ToTIYUV420PackedSemiPlanar(srcPtr, dstPtr, w, h, &dstSize);


      /*
      ColorConverter colorConverter(OMX_COLOR_Format32bitARGB8888, OMX_TI_COLOR_FormatYUV420PackedSemiPlanar);
      err = colorConverter.convert(srcPtr,
                              w, h,
                              0, 0, w-1, h-1, //crop
                              dstPtr,
                              w, h,
                              0, 0, w-1, h-1);
      */


  }

  return err;
}

class CaptureScreenShotThread : public Thread {
public:
  CaptureScreenShotThread(const sp<IBinder> &display, const sp<MediaCodec> encoder)
  : mDisplay(display),
    mFramePeriodMicroSec(1000000 / gVideoFPS),
    mEncoder(encoder),
    mRunning(false) {


  }

  bool threadLoop() {

    status_t err;
    size_t inputBufferId;
    size_t bufSize;
    struct timespec now;

    if(!mRunning) {
      clock_gettime(CLOCK_MONOTONIC, &now);
      mStartTime = now;

      err = mEncoder->getInputBuffers(&mInputBuffers);
      if(err != NO_ERROR) {
        printf("error: %d", err);
        return false;
      }

      mRunning = true;
    }

    err = mEncoder->dequeueInputBuffer(&inputBufferId, DEFAULT_TIMEOUT);
    if(err == OK) {
      sp<ABuffer> inputBuffer = mInputBuffers[inputBufferId];
      err = screenShotToBuffer(mDisplay, inputBuffer, bufSize);

      clock_gettime(CLOCK_MONOTONIC, &now);


      mPresentationTimeUs = toMicroSec(now) - toMicroSec(mStartTime);

      mEncoder->queueInputBuffer(inputBufferId, 0, bufSize, mPresentationTimeUs, 0);
      mFrameNum++;


      int64_t sleepTime = mFramePeriodMicroSec*mFrameNum - mPresentationTimeUs;
      if(sleepTime > 0) {
        usleep(sleepTime);
      }
    }

    return mRunning;
  }

private:
  sp<IBinder> mDisplay;
  const uint64_t mFramePeriodMicroSec;
  sp<MediaCodec> mEncoder;
  struct timespec mStartTime;
  int64_t mPresentationTimeUs;
  uint32_t mFrameNum;
  bool mRunning;
  Vector< sp<ABuffer> > mInputBuffers;

  static uint64_t toMicroSec(const struct timespec &time) {
      return time.tv_sec*1000000 + time.tv_nsec / 1000;
  }


};

int main() {

  status_t err;

  sp<ProcessState> self = ProcessState::self();
  self->startThreadPool();

  int32_t displayId = DEFAULT_DISPLAY_ID;
  sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(displayId);

  DisplayInfo displayInfo;
  SurfaceComposerClient::getDisplayInfo(display, &displayInfo);
  gVideoWidth = displayInfo.w;
  gVideoHeight = displayInfo.h;

  sp<MediaCodec> encoder;
  err = prepareEncoder(encoder);
  if(err != NO_ERROR) {
    printf("error: %d", err);
    return err;
  }

  err = encoder->start();
  if(err != NO_ERROR) {
    printf("error: %d", err);
    return err;
  }

  Vector< sp<ABuffer> > inputBuffers;
  Vector< sp<ABuffer> > outputBuffers;

  err = encoder->getInputBuffers(&inputBuffers);
  if(err != NO_ERROR) {
    printf("error: %d", err);
    return err;
  }
  err = encoder->getOutputBuffers(&outputBuffers);
  if(err != NO_ERROR) {
    printf("error: %d", err);
    return err;
  }

  size_t inputBufferId;
  int64_t presentationTimeUs = 0;

  for(int i=0;i<50;i++) {

    err = encoder->dequeueInputBuffer(&inputBufferId, DEFAULT_TIMEOUT);
    /*err could be:
     OK
     -EAGAIN aka DEQUEUE_INFO_TRY_AGAIN_LATER
     INFO_FORMAT_CHANGED
     INFO_OUTPUT_BUFFERS_CHANGED
    */
    if(err == OK) {
      size_t dstSize;

      sp<ABuffer> inputBuffer = inputBuffers[inputBufferId];
      err = screenShotToBuffer(display, inputBuffer, dstSize);

      encoder->queueInputBuffer(inputBufferId, 0, dstSize, presentationTimeUs, 0);

    }




  }




  return 0;
}
