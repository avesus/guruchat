# ifndef __DEVICE_H__
# define __DEVICE_H__

// Enumerate audio inputs
// List of current audio inputs.
// Fallback, позволяющий автоматически переключиться на другой аудиовход, если текущий становится недоступен (при отключении устройства).
// Чтобы такой fallback работал, необходима виртуализация групп источников.
// Имеет смысл, когда записывается только один канал.
// При многоканальной записи обрабатывается обычное отключение.

// Понятие перечисления не прикольно. Прикольнее список текущих входов.
// Управление всей этой периферией и каналами будет весьма затратным занятием.
// Если что-то отключается, движок захвата должен получить соотв. сообщение.
// Если появляется новое устройство, движок также должен об этом узнать.

// Устройства могут быть новые и старые. Для старых это просто "возвращение в строй",
// а новые должны присылать именно событие "новое".

// Если настроек сопоставления устройств группам захвата нет, движок получает события
// "новые устройства" аналогичным перечислению способом.

// Таким образом, движок в рантайме имеет список текущих устройств и сопоставленных
// им настроек. При повторном запуске программы этот список загружается
// из настроек и генерит сообщение "наше устройство теперь доступно".

// Дифференциация недоступного, удаленного и отключенного устройств не делается,
// для простоты.

// Итог:
// NEW, LOADED, UNLOADED

// При отображении списка устройств отображалка ПОДПИСЫВАЕТСЯ на данные уведомления.
// Чтобы отображалка получила каждый элемент, она должна получить 2 элемента:
// 1. Контейнер. Он будет слать только уведомление NEW.
// 2. Элементы в этом контейнере. Объекты. Доступны после того, как клиент подписался
// на уведомления NEW контейнера. Интерфейс отображения может сохранять отдельно
// свои специальные настройки, ассоциированные с элементами контейнера.
// Желательно, чтобы интерфейс мог сопоставлять их и ассоциировать при получении событий LOADED/UNLOADED.
// Возможность одновременной записи из нескольких источников.

// Enumerate audio outputs

// Источник должен запоминать, какие элементы были переданы наблюдателю.
// Если какие-то элементы появились, а потом исчезли, пока наблюдатель не был
// подписан, должна быть политика, позволяющая использовать
// оба режима (последнее состояние или цепочку состояний).
// Вопрос в том, зачем оно нужно. Так как мы строим абстракцию актуальных
// списков, то состояние будет доступно в обход, по другим интерфейсам.
// И история изменений состояния будет трекаться также по другим интерфейсам,
// только там, где оно действительно нужно.
// Итого: при подключении наблюдателя он должен получить текущий список элементов.
// Если наблюдатель раньше уже подключался, он должен на известные ему элементы
// получить уведомления "LOADED", на неизвестные - "NEW".
// Все элементы, которые переданы, в любой момент могут получить сообщение "UNLOADED".

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

// VideoCaptureSources - список видеокамер, в том числе и отключенных
// VideoPlaybackDestinations - список дисплеев, в том числе и отключенных
// AudioCaptureSources - список устройств захвата звука, в том числе отсутствующих
// AudioPlaybackDestinations - список устройств

class IDevice;
class IDeviceInfo;
class IDeviceSupply;

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