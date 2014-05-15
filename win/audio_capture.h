# ifndef __AUDIO_CAPTURE_H__
# define __AUDIO_CAPTURE_H__

# include "device.h"

class AudioCapture : public IDeviceSupply
{
public:
  AudioCapture();
  virtual ~AudioCapture();
};

# endif
