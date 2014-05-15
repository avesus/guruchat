# ifndef __DEVICE_H__
# define __DEVICE_H__

// Enumerate audio inputs
// List of current audio inputs.
// Fallback, ����������� ������������� ������������� �� ������ ���������, ���� ������� ���������� ���������� (��� ���������� ����������).
// ����� ����� fallback �������, ���������� ������������� ����� ����������.
// ����� �����, ����� ������������ ������ ���� �����.
// ��� �������������� ������ �������������� ������� ����������.

// ������� ������������ �� ���������. ���������� ������ ������� ������.
// ���������� ���� ���� ���������� � �������� ����� ������ ��������� ��������.
// ���� ���-�� �����������, ������ ������� ������ �������� �����. ���������.
// ���� ���������� ����� ����������, ������ ����� ������ �� ���� ������.

// ���������� ����� ���� ����� � ������. ��� ������ ��� ������ "����������� � �����",
// � ����� ������ ��������� ������ ������� "�����".

// ���� �������� ������������� ��������� ������� ������� ���, ������ �������� �������
// "����� ����������" ����������� ������������ ��������.

// ����� �������, ������ � �������� ����� ������ ������� ��������� � ��������������
// �� ��������. ��� ��������� ������� ��������� ���� ������ �����������
// �� �������� � ������� ��������� "���� ���������� ������ ��������".

// �������������� ������������, ���������� � ������������ ��������� �� ��������,
// ��� ��������.

// ����:
// NEW, LOADED, UNLOADED

// ��� ����������� ������ ��������� ����������� ������������� �� ������ �����������.
// ����� ����������� �������� ������ �������, ��� ������ �������� 2 ��������:
// 1. ���������. �� ����� ����� ������ ����������� NEW.
// 2. �������� � ���� ����������. �������. �������� ����� ����, ��� ������ ����������
// �� ����������� NEW ����������. ��������� ����������� ����� ��������� ��������
// ���� ����������� ���������, ��������������� � ���������� ����������.
// ����������, ����� ��������� ��� ������������ �� � ������������� ��� ��������� ������� LOADED/UNLOADED.
// ����������� ������������� ������ �� ���������� ����������.

// Enumerate audio outputs

// �������� ������ ����������, ����� �������� ���� �������� �����������.
// ���� �����-�� �������� ���������, � ����� �������, ���� ����������� �� ���
// ��������, ������ ���� ��������, ����������� ������������
// ��� ������ (��������� ��������� ��� ������� ���������).
// ������ � ���, ����� ��� �����. ��� ��� �� ������ ���������� ����������
// �������, �� ��������� ����� �������� � �����, �� ������ �����������.
// � ������� ��������� ��������� ����� ��������� ����� �� ������ �����������,
// ������ ���, ��� ��� ������������� �����.
// �����: ��� ����������� ����������� �� ������ �������� ������� ������ ���������.
// ���� ����������� ������ ��� �����������, �� ������ �� ��������� ��� ��������
// �������� ����������� "LOADED", �� ����������� - "NEW".
// ��� ��������, ������� ��������, � ����� ������ ����� �������� ��������� "UNLOADED".

// Datastream
// DatastreamAudio
// DatastreamVideo
// DatastreamIn
// DatastreamOut

// AudioSrc
// AudioDst
// VideoSrc
// VideoDst

// VideoSrcPool

// VideoCaptureSources - ������ ����������, � ��� ����� � �����������
// VideoPlaybackDestinations - ������ ��������, � ��� ����� � �����������
// AudioCaptureSources - ������ ��������� ������� �����, � ��� ����� �������������
// AudioPlaybackDestinations - ������ ���������

class IDevice;
class IDeviceInfo;

enum EOnOffBehavior
{
  ONOFFBEHAVIOR_IGNORE,
  ONOFFBEHAVIOR_PUSH_POP,
  ONOFFBEHAVIOR_ON_OFF
};

class IDeviceConsumer
{
public:
  // Called for backward connection with device supplier.
  // Device could be turned off and available. Called when app starts.
  virtual void onDeviceConnected(IDeviceSupply* supplier) = 0;

  // Called back when enumDevices() called.
  virtual void onDeviceListed(const IDeviceInfo* device) = 0;

  // IDevice ownself doesn't became the device, so IDeviceInfo supplied.
  virtual EOnOffBehavior onNewDevice(const IDeviceInfo* device) = 0;

  // Interesting because of possible format change.
  // Called back when 'STACK' returned from onNewDevice
  virtual void onDeviceChanged(IDeviceSupply* currentDevice) = 0;
  // Called back when threre is no any device then connected a device
  // or 'ONOFF' returned from onNewDevice
  virtual void onDeviceEnabled(IDeviceSupply* enabledDevice) = 0;
  // Called back when no active devices left or 'ONOFF' returned from onNewDevice
  virtual void onDeviceDisabled(IDeviceSupply* disabledDevice) = 0;
};

class IDeviceInfo
{
public:
  virtual const wchar_t* name() const = 0;
  virtual bool isEnabled() const = 0;
};

class IDeviceSupply : public IDeviceInfo
{
public:
  // Enabled/disabled device settings are per subscriber.
  // If consumer id is unknown, supply 0 - it will be assigned by supplier.
  virtual int consumeDevice(IDeviceConsumer* consumer, int consumerId) = 0;

  virtual void enumDevices(IDeviceConsumer* consumer) = 0;

  // The method returns false if newDevice is disabled, forces enabling newDevice
  // and moves previous device in stack.
  // If previous device was turned off, sends enabled device signal
  // Device selections are per consumer, so, this supply is different for different
  // consumers.
  virtual bool changeDevice(IDeviceConsumer* consumer,
    const IDeviceInfo* newDevice) = 0;

  // The method returns false if no devices, error or 'STACK' policy
  // for auto-changing devices is not used.
  virtual bool enableDevice(IDeviceConsumer* consumer) = 0;
  virtual void disableDevice(IDeviceConsumer* consumer) = 0;
};

class IPersistSettings
{
public:

};

class ISettingsStore
{
public:
  virtual IPersistSettings* settingsFor(const char* componentId) = 0;
};

/*
void foo()
{
  class AudioCaptureSources : public IDeviceSupply
  {
  };

  class SelectMicrophoneGui : public IDeviceConsumer
  {
  };

  SelectMicrophoneGui g;
  AudioCaptureSources s;

  s

  ISettingsStore* store = 0;

  SourceDevices audioSrc(store->settingsFor("audio_src"));
  //DeviceChannel* ch0 = audioSrc.getChannel("ch0");
  DeviceChannel ch1;
  audioSrc.createChannel("ch1");

  IExternalObserver* encodersEngine = 0;

  ch1.setExternalObserver(encodersEngine);
}
*/

# endif