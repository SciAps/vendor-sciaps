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

#include <media/openmax/OMX_IVCommon.h>

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
  format->setInt32("color-format", OMX_COLOR_FormatAndroidOpaque);
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

int main() {

  status_t err;

  sp<ProcessState> self = ProcessState::self();
  self->startThreadPool();

  int32_t displayId = DEFAULT_DISPLAY_ID;

  void const* base = 0;
  uint32_t w, h, f;
  size_t size = 0;

  ScreenshotClient screenshot;
  sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(displayId);

  DisplayInfo displayInfo;
  SurfaceComposerClient::getDisplayInfo(display, &displayInfo);
  gVideoWidth = displayInfo.w;
  gVideoHeight = displayInfo.h;

  if (display != NULL && screenshot.update(display) == NO_ERROR) {
      base = screenshot.getPixels();
      w = screenshot.getWidth();
      h = screenshot.getHeight();
      f = screenshot.getFormat();
      size = screenshot.getSize();

      printf("w: %d h: %d format: %d\n", w, h, f);

  } else {
    printf("error getting display\n");
  }

  sp<MediaCodec> encoder;
  err = prepareEncoder(encoder);
  if(err != NO_ERROR) {
    printf("error: %d", err);
    return err;
  }



  return 0;
}
