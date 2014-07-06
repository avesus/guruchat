# include "./../nanosec.h"

#pragma comment(lib, "winmm")
# include <stdio.h>

static int CP[64] = {0};
static int k = 2;

static uint64_t t_r1 = 0;
static uint64_t t_s1 = 0;

static uint64_t tau_r[64] = {0};
static uint64_t tau_s[64] = {0};
static double delta_t_rs_med = 0;
static double delta_t_rs[64] = {0};
static double delta_T_rs[64] = {0};
static double diff[64] = {0};
static double kaldiff[64] = {0};
static int i = 0;

struct kalman_state
{
  double q; //process noise covariance
  double r; //measurement noise covariance
  double x; //value
  double p; //estimation error covariance
  double k; //kalman gain
};

kalman_state kalman = {0};

// difference in clocks
// firstly estimated as tr[0] - ts[0] ????
// then estimated with Kalman filtering
static double alpha = 1.0;//000001;//0.99999;//0.99995;

// official delay.
// firstly. roughly estimated as RTT/2
// then, soft adapted with new packets
//static double beta = 915;//-49950;//1000.0;

double congestionDelayThreshold = 1000.0;

void init_counter()
{
  CP[1] = 1;
  CP[2] = 2;
  k = 2;

  i = 0;

  //process noise covariance
  kalman.q = 0.25;
  //measurement noise covariance
  kalman.r = 0.7;
  

  kalman.x = 0.0;

  //estimation error covariance
  kalman.p = 0.4;

  kalman.k = 0.445;
}


double s(int i1, int i2)
{
  if (tau_r[i2] == tau_r[i1])
  {
    return (delta_t_rs[i2] - delta_t_rs[i1]) * 999999999999LL;//0;// * 99999999999LL;
  }
  return double(delta_t_rs[i2] - delta_t_rs[i1])/double(int64_t(tau_r[i2]) - int64_t(tau_r[i1]));
}

double f(int i1, int i2, uint64_t t)
{
  return s(i1, i2) * double(int64_t(t) - int64_t(tau_r[i1])) + double( delta_t_rs[i1] );
}

uint64_t minArrivalInterpacket = 1;
uint64_t maxArrivalInterpacket = 1000000000;

# if 0
void on_data_packet0(uint32_t loopedPacketSeqNum, uint64_t senderNanosecsTime,
                    uint32_t payloadSize)
{
  uint64_t t_r = Nanosec::Now();
  uint64_t t_s = senderNanosecsTime;

  ++i;
  
  if (i == 1)
  {
    t_r1 = t_r;
    t_s1 = t_s;
  }

  tau_r[i] = t_r - t_r1;
  tau_s[i] = t_s - t_s1;

  delta_t_rs[i] = tau_r[i] - tau_s[i];

  if (i < 3)
  {
    return;
  }

  uint64_t arrivalInterpacket = tau_r[i] - tau_r[i-1];
  if (arrivalInterpacket < minArrivalInterpacket
   || arrivalInterpacket > maxArrivalInterpacket)
  {
    return;
  }

  double f1 = f(CP[k-1], CP[k], tau_r[i]);
  double f2 = f(CP[k], CP[k+1], tau_r[i]);

  if (delta_t_rs[i] < f1)
  {
    // find l for:
    for(int l = i-2; l >= 2 && l >= i-16; --l)
    {
      double v1 = s(CP[l-1], CP[l]);
      double v2 = s(CP[l-1], i);
      double v3 = s(CP[l], CP[l+1]);
      if (v1 <= v2 && v2 < v3)
      {
        CP[l] = i;
        k = l;
        break;
      }
    }
  }
  else if (CP[k] == i-1)
  {
    CP[k+1] = i;
  }
  else if (delta_t_rs[i] < f2)
  {
    CP[k+1] = i;
  }
  //else
 
  //if (delta_t_rs[CP[k]] < (tau_r[i] - tau_r[CP[k]]))
  //if (tau_r[CP[k]] < (tau_r[i] - tau_r[i-1]))
  //if (delta_t_rs[CP[k+1]] < (tau_r[i] - tau_r[CP[k]]))
  //int diff = tau_r[i] - tau_r[CP[k]];
  if (tau_r[CP[k]] < tau_r[i])//diff)
  //
  {
    ++k;
    CP[k+1] = i;
    //++k;
  }
  //else
  //{
  //  int uu = 5;
  //}

  // f(CP[k-1], CP[k], tau_r[i]) = a * tau_r[i] - b

  delta_T_rs[i] = double(delta_t_rs[i]) - f(CP[k-1], CP[k], tau_r[i]);

  printf("%lld\r\n", delta_T_rs[i]);
}
# endif

static double med_delay_change_velocity = 0;

void on_data_packet(uint32_t loopedPacketSeqNum, uint64_t senderNanosecsTime,
                    uint32_t payloadSize);

void calculate()
{
  timeBeginPeriod(1);

  Nanosec::Now();

  // Фильтр Калмана для отсчетов - его ТЕКУЩЕЕ ЗНАЧЕНИЕ
  // даст текущий режим передачи пакетов - текущую задержку.

  // Производная функции после Калмана как раз даст скорость ухудшения/улучшения.
  // Когда скорость ухудшения/улучшения за заданный интервал времени будет превышать какое-то
  // значение (не СУММА отсчетов скорости, а именно текущее значение скорости именно за интервал
  // времени), то отправителию говорится, что он "шалит"
  // (до первого drop'а из-за congestion'а!).
  // Возврат же реализуется по-другому: за заданный интервал времени результат Калмана
  // меряется и отправляется минимальное и максимальное значение. Когда отправитель видит,
  // что текущая задержка маленькая, он может попытаться повысить скорость передачи.
  // Когда отправитель пропускает ответы получателя, важно, чтобы он по следующему
  // ответу принял правильное решение.
  // Так как у нас UDP, то для надежности ответы можно слать с каждым обратным пакетом.
  //

  for(int i = 0; i < 60; ++i)
  {
//    ::Sleep(1);
    /*for(int a = 0; a < 100000; ++a)
    {
      int u = 0;
      int k = 5;
      memcpy(&k, &u, 4);
    }*/
    uint64_t t_s = Nanosec::Now() + 3257049857237;
    //int j = (i > 10 && i < 20) ? i-10 : 0;
    //::Sleep(i);//i*10);
    if(i<15 || i>37) {
      ::Sleep(50);
      /*for(int a = 0; a < 10000 * (1); ++a)
      {
        int u = 0;
        int k = 5;
        memcpy(&k, &u, 4);
      }*/
    } else {
      ::Sleep(i);
      /*for(int a = 0; a < 510000 * (1); ++a)
      {
        int u = 0;
        int k = 5;
        memcpy(&k, &u, 4);
      }*/
    }
    /*
    for(int a = 0; a < 510000 * (1); ++a)
    {
      int u = 0;
      int k = 5;
      memcpy(&k, &u, 4);
    }*/
    on_data_packet(0, t_s, 0);
    ::Sleep(50);
  }
}

void on_data_packet(uint32_t loopedPacketSeqNum, uint64_t senderNanosecsTime,
                    uint32_t payloadSize)
{
  uint64_t t_r = Nanosec::Now();
  uint64_t t_s = senderNanosecsTime;

  tau_r[i] = t_r;
  tau_s[i] = t_s;

  if(!i)
  {
    delta_t_rs[i] = 50000000 - 75000;// - 830000 + 450000 - 80000;//beta;
    delta_t_rs_med = delta_t_rs[i];//beta;
    ++i;
    return;
  }

  static double med_dt = 0;
  med_dt = 0.5 * (med_dt + delta_t_rs[i]);

  delta_t_rs[i] = double(tau_r[i] - tau_r[i-1]) - alpha * double(tau_s[i] - tau_s[i-1])
    //+ med_dt;
    + double(delta_t_rs[i-1]);
    //+ beta;

  double prevX = kalman.x;

  kalman.p = kalman.p + kalman.q;

  //measurement update
  kalman.k = kalman.p / (kalman.p + kalman.r);
  kalman.x = kalman.x + kalman.k * ((delta_t_rs[i] - delta_t_rs[i-1]) - kalman.x);
  kalman.p = (1.0 - kalman.k) * kalman.p;

  //beta = 0.5 * (beta + delta_t_rs[i]);

  //delta_t_rs_med = (delta_t_rs_med + delta_t_rs[i]) / 2;

  delta_T_rs[i] = (delta_t_rs[i] - delta_t_rs[i-1]);//kalman.x;// - prevX;//0.001 * (delta_t_rs[i] - delta_t_rs[i-1]);//2 * beta;//delta_t_rs_med;

  double pos_sum = 0;
  for (int u = i; u && u > i-6; --u)
  {
    if (delta_T_rs[u] > congestionDelayThreshold)
    {
      pos_sum += 1;//delta_T_rs[u];
    }
    else
    {
      pos_sum = 0;
    }
  }

  diff[i] = pos_sum;

  //diff[i] = (delta_T_rs[i] - delta_T_rs[i-1]);

  //med_delay_change_velocity += kalman.x;
  //diff[i] = med_delay_change_velocity / 2.0;

  int tot = 0;
  static double min_delta = 1000000000000.0;
  static double max_delta = -1000000000000.0;
  static double max_abs_delta = 0.0;
  int j = i;
  //for (int j = i; j >= i-6 && j > 0; --j, ++tot)
  //{
    if (delta_T_rs[j] > max_delta)
    {
      max_delta = delta_T_rs[j];
    }

    if (delta_T_rs[j] < min_delta)
    {
      min_delta = delta_T_rs[j];
    }

    double curr_delta = max_delta - min_delta;
    if (curr_delta > max_abs_delta)
    {
      max_abs_delta = curr_delta;
    }

    //diff[i] = max_abs_delta;//+= delta_T_rs[j];
  //}

  double absD = delta_T_rs[i] > 0.0 ? delta_T_rs[i] : -delta_T_rs[i];

  double sign = absD/delta_T_rs[i];

//  diff[i] = (absD / max_abs_delta) > 0.45 ? sign : 0.0;
//  if (diff[i])
//    diff[i] += diff[i-1];

/*  diff[i] += diff[i-1];

  diff[i] /= double(tot+1);
  //diff[i];// -= diff[i-1];

  kalman.x = kalman.x + kalman.k * (diff[i] - kalman.x);
  kalman.p = (1.0 - kalman.k) * kalman.p;

  kaldiff[i] = kalman.x;
*/
  ++i;
} 