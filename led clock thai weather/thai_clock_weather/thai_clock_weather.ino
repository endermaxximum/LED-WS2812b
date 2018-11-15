#include <ESP8266WiFi.h>
#include <JsonListener.h>
#include <WundergroundConditions.h>
#include "MD_MAX72xx.h"
#include <time.h>

#define WIFI_SSID     "ssid"
#define WIFI_PASSWORD "passw0rd"
char ntp_server1[20] = "ntp.ku.ac.th";
char ntp_server2[20] = "fw.eng.ku.ac.th";
char ntp_server3[20] = "time.uni.net.th";
int timezone = 7;
int dst = 0;
unsigned long curTime, changeMode;
int tmy, tmm, tmd, tmw, dots, mode;
String tms = "";

// Wunderground Settings : https://www.wunderground.com/weather/api/
const int UPDATE_INTERVAL_SECS = 10 * 60; // Update every 10 minutes
bool IS_METRIC = true;
const String WUNDERGRROUND_API_KEY = "you wunderground api key";
const String WUNDERGRROUND_LANGUAGE = "EN";
const String WUNDERGROUND_COUNTRY = "Thailand";
const String WUNDERGROUND_CITY = "Samui";  // you city
long lastDownloadUpdate = millis();
WGConditions conditions;

char* weekdayNames[] = {"อาทิตย์", "จันทร์", "อังคาร", "พุธ", "พฤหัสบดี", "ศุกร์", "เสาร์"};
char* monthNames[] = {"มกราคม", "กุมภาพันธ์", "มีนาคม", "เมษายน", "พฤษภาคม", "มิถุนายน",
                      "กรกฎาคม", "สิงหาคม", "กันยายน", "ตุลาคม", "พฤศจิกายน", "ธันวาคม"};

int g_pcman_char = 0;
int g_pcman_n = 0;

#define MAX_DEVICES 16  //32
#define CLK_PIN   D5
#define CS_PIN    D8
#define DATA_PIN  D7

MD_MAX72XX mx = MD_MAX72XX(CS_PIN, MAX_DEVICES);

int TD_max_row = 16;
int TD_max_col = (MAX_DEVICES / 8) * 32;
int TD_max_char_row1 = 16;
int TD_max_char_row2 = 12;
int TD_max_char_col = 16;
int TD_normal_row = 1;
int TD_start_not_thai_char = 33;
int TD_start_thai_char = 161;
int TD_gap_pixel = 2;
int TD_led_delay = 20;

// thai over
const int char_char_x1 = 13;
int char_x1[char_char_x1] = { 209, 212, 213, 214, 215, 231, 232, 233, 234, 235, 236, 237, 238 };

// thai under
const int char_char_x2 = 2;
int char_x2[char_char_x2] = { 216, 217 };

unsigned int char_data1[94][16] = {
  {61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 0, 0, 24576, 61440, 61440, 24576}, //  33  !
  {55296, 55296, 18432, 18432, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //  34 "
  {0, 0, 0, 0, 2176, 2176, 32736, 4352, 4352, 4352, 65472, 8704, 8704, 0, 0, 0}, //  35  #
  {0, 0, 0, 8192, 28672, 43008, 40960, 24576, 12288, 10240, 43008, 28672, 8192, 0, 0, 0}, //  36  $
  {0, 0, 0, 1536, 1024, 52224, 51200, 6144, 4096, 13824, 9728, 24576, 49152, 0, 0, 0}, //  37  %
  {0, 0, 0, 0, 28672, 55296, 34816, 56064, 29184, 56320, 35840, 55808, 29440, 0, 0, 0}, //  38  &
  {49152, 49152, 49152, 49152, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //  39  '
  {0, 0, 0, 6144, 12288, 24576, 49152, 49152, 49152, 49152, 24576, 12288, 6144, 0, 0, 0}, //  40  (
  {0, 0, 0, 49152, 24576, 12288, 6144, 6144, 6144, 6144, 12288, 24576, 49152, 0, 0, 0}, //  41  )
  {0, 0, 0, 0, 6144, 23040, 65280, 32256, 15360, 32256, 59136, 16896, 0, 0, 0, 0}, //  42  *
  {0, 0, 0, 0, 6144, 6144, 6144, 65280, 65280, 6144, 6144, 6144, 0, 0, 0, 0}, //  43  +
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12288, 28672, 57344, 49152}, //  44 ,
  {0, 0, 0, 0, 0, 0, 0, 64512, 64512, 0, 0, 0, 0, 0, 0, 0}, //  45  -
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24576, 61440, 61440, 24576}, //  46  .
  {0, 0, 0, 512, 1536, 3072, 3072, 6144, 6144, 12288, 12288, 24576, 16384, 0, 0, 0}, //  47  /
  {0, 16128, 32640, 57792, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 57792, 32640, 16128, 0}, //  48  0
  {0, 3072, 7168, 15360, 15360, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 16128, 16128, 0},    //  49  1
  {0, 16128, 32640, 57792, 49344, 192, 448, 896, 1792, 3584, 7168, 14336, 28672, 65472, 65472, 0},   //  50  2
  {0, 16128, 32640, 57792, 49344, 192, 192, 1920, 1920, 192, 192, 49344, 57792, 32640, 16128, 0},    //  51  3
  {0, 768, 1792, 3840, 7936, 15104, 29440, 58112, 58112, 65472, 65472, 768, 768, 768, 768, 0},       //  52  4
  {0, 65472, 65472, 57344, 57344, 65280, 65408, 960, 448, 448, 448, 57792, 62400, 32640, 16128, 0},  //  53  5
  {0, 16128, 32640, 57792, 57344, 57344, 65280, 65408, 58304, 57792, 57792, 57792, 62400, 32640, 16128, 0}, //  54  6
  {0, 65472, 65472, 448, 960, 1920, 3840, 7680, 15360, 14336, 14336, 14336, 14336, 14336, 14336, 0}, //  55  7
  {0, 16128, 32640, 57792, 49344, 49344, 24960, 16128, 32640, 62400, 57792, 57792, 62400, 32640, 16128, 0}, //  56  8
  {0, 16128, 32640, 57792, 49344, 49344, 57792, 32704, 16320, 192, 192, 192, 448, 16256, 16128, 0},  //  57  9
  //{0, 0, 0, 24576, 61440, 61440, 24576, 0, 0, 24576, 61440, 61440, 24576, 0, 0, 0}, //  58  :
  {0, 0, 12288, 30720, 64512, 30720, 12288, 0, 0, 12288, 30720, 64512, 30720, 12288, 0, 0}, //  :
  {0, 0, 0, 24576, 61440, 61440, 24576, 0, 28672, 28672, 61440, 57344, 49152, 0, 0, 0}, //  59  ;
  {0, 0, 0, 3072, 6144, 12288, 24576, 49152, 49152, 24576, 12288, 6144, 3072, 0, 0, 0}, //  60  <
  {0, 0, 0, 0, 0, 0, 64512, 64512, 0, 64512, 64512, 0, 0, 0, 0, 0}, //  61  =
  {0, 0, 0, 49152, 24576, 12288, 6144, 3072, 3072, 6144, 12288, 24576, 49152, 0, 0, 0}, //  62  >
  {0, 0, 0, 30720, 64512, 52224, 3072, 6144, 12288, 12288, 0, 12288, 12288, 0, 0, 0}, //  63  ?
  {0, 0, 0, 14336, 17408, 33280, 37376, 43520, 43520, 43520, 37888, 16384, 14336, 0, 0, 0}, //  64  @
  {0, 16128, 32640, 57792, 49344, 49344, 49344, 49344, 49344, 65472, 65472, 49344, 49344, 49344, 49344, 0}, //  65  A
  {0, 65280, 65408, 49600, 49344, 49344, 49600, 65408, 65408, 49600, 49344, 49344, 49600, 65408, 65280, 0}, //  66  B
  {0, 16128, 32640, 57792, 49344, 49152, 49152, 49152, 49152, 49152, 49152, 49344, 57792, 32640, 16128, 0}, //  67  C
  {0, 65280, 65408, 49600, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 49600, 65408, 65280, 0}, //  68  D
  {0, 65472, 65472, 49152, 49152, 49152, 49152, 65280, 65280, 49152, 49152, 49152, 49152, 65472, 65472, 0}, //  69  E
  {0, 65472, 65472, 49152, 49152, 49152, 49152, 65280, 65280, 49152, 49152, 49152, 49152, 49152, 49152, 0}, //  70  F
  {0, 16128, 32640, 57792, 49344, 49152, 49152, 50112, 50112, 49344, 49344, 49344, 57792, 32640, 16128, 0}, //  71  G
  {0, 49344, 49344, 49344, 49344, 49344, 49344, 65472, 65472, 49344, 49344, 49344, 49344, 49344, 49344, 0}, //  72  H
  {0, 16128, 16128, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 16128, 16128, 0},    //  73  I
  {0, 65472, 65472, 192, 192, 192, 192, 192, 192, 192, 192, 49344, 57792, 32640, 16128, 0},          //  74  J
  {0, 49344, 49600, 50048, 50944, 52736, 64512, 63488, 63488, 64512, 52736, 50944, 50048, 49600, 49344, 0}, //  75  K
  {0, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 65472, 65472, 0}, //  76  L
  {0, 49344, 57792, 62400, 65472, 57024, 52416, 52416, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 0}, //  77  M
  {0, 49344, 57536, 57536, 61632, 53440, 55488, 51392, 52416, 50368, 50880, 49856, 50112, 49600, 49344, 0}, //  78  N
  {0, 16128, 32640, 57792, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 57792, 32640, 16128, 0}, //  79  O
  {0, 65280, 65408, 50112, 49600, 49600, 50112, 65408, 65280, 49152, 49152, 49152, 49152, 49152, 49152, 0}, //  80  P
  {0, 16128, 32640, 49600, 49344, 49344, 49344, 49344, 49344, 49344, 50880, 51136, 50048, 32704, 16064, 0}, //  81  Q
  {0, 65280, 65408, 49600, 49344, 49344, 49600, 65408, 65280, 56320, 52736, 50944, 50048, 49600, 49344, 0}, //  82  R
  {0, 16256, 32704, 57536, 49152, 49152, 57344, 32512, 16256, 448, 192, 192, 49600, 65408, 32512, 0}, //  83  S
  {0, 65472, 65472, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 0},      //  84  T
  {0, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 57792, 32640, 16128, 0}, //  85  U
  {0, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 57792, 29568, 16128, 7680, 3072, 0}, //  86  V
  {0, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 52416, 52416, 57024, 65472, 32640, 13056, 0}, //  87  W
  {0, 49344, 49344, 49344, 57792, 29568, 15872, 7168, 7680, 16128, 29568, 57792, 49344, 49344, 49344, 0}, //  88  X
  {0, 49344, 49344, 49344, 57792, 29568, 16128, 7680, 3072, 3072, 3072, 3072, 3072, 3072, 3072, 0},  //  89  Y
  {0, 65472, 65472, 192, 448, 896, 1792, 3584, 7168, 14336, 28672, 57344, 49152, 65472, 65472, 0},   //  90  Z
  {0, 0, 0, 57344, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 57344, 0, 0, 0}, //  91 [
  {0, 0, 0, 32768, 49152, 16384, 24576, 8192, 4096, 6144, 2048, 3072, 1024, 0, 0, 0}, //  92
  {0, 0, 0, 57344, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 8192, 57344, 0, 0, 0}, //  93  ]
  {4096, 14336, 27648, 50688, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //  94  ^
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65280, 65280}, //  95  _
  {49152, 24576, 12288, 6144, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //  96  `
  {0, 0, 0, 0, 0, 16128, 32640, 25024, 192, 16320, 32704, 49344, 49344, 32704, 16256, 0}, //  97  a
  {0, 49152, 49152, 49152, 49152, 65280, 65408, 49600, 49344, 49344, 49344, 49344, 49600, 65408, 65280, 0}, //  98  b
  {0, 0, 0, 0, 0, 16128, 32640, 57792, 49344, 49152, 49152, 49344, 57792, 32640, 16128, 0}, //  99 c
  {0, 192, 192, 192, 192, 16320, 32704, 57536, 49344, 49344, 49344, 49344, 57536, 32704, 16320, 0}, //  100 d
  {0, 0, 0, 0, 0, 16128, 32640, 57792, 49344, 65472, 65408, 57344, 57792, 32640, 16128, 0}, //  101 e
  {0, 6144, 15360, 32256, 26112, 24576, 24576, 63488, 63488, 24576, 24576, 24576, 24576, 24576, 24576, 0}, //  102 f
  {0, 0, 0, 0, 0, 16320, 32704, 57536, 49344, 57536, 32704, 16320, 192, 58304, 32640, 16128}, //  103 g
  {0, 49152, 49152, 49152, 49152, 65280, 65408, 49600, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 0}, //  104 h
  {0, 0, 49152, 49152, 0, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 0}, //  105 i
  {0, 768, 768, 0, 0, 768, 768, 768, 768, 768, 768, 49920, 59136, 32256, 15360, 0}, //  106 j
  {0, 49152, 49152, 49152, 49152, 49920, 50944, 52736, 56320, 63488, 63488, 63488, 56320, 52736, 50688, 0}, //  107 k
  {0, 61440, 61440, 12288, 12288, 12288, 12288, 12288, 12288, 12288, 12288, 12288, 12288, 64512, 64512, 0}, //  108  l
  {0, 0, 0, 0, 0, 65408, 65472, 52416, 52416, 52416, 52416, 52416, 52416, 52416, 52416, 0}, //  109  m
  {0, 0, 0, 0, 0, 52992, 57216, 63936, 61632, 49344, 49344, 49344, 49344, 49344, 49344, 0}, //  110  n
  {0, 0, 0, 0, 0, 16128, 32640, 57792, 49344, 49344, 49344, 49344, 57792, 32640, 16128, 0}, //  111 o
  {0, 0, 0, 0, 0, 65280, 65408, 49600, 49344, 49344, 49600, 65408, 65280, 49152, 49152, 49152}, //  112 p
  {0, 0, 0, 0, 0, 16320, 32704, 57536, 49344, 49344, 57536, 32704, 16320, 192, 192, 192}, //  113 q
  {0, 0, 0, 0, 0, 52992, 57216, 63936, 61632, 57344, 49152, 49152, 49152, 49152, 49152, 0}, //  114 r
  {0, 0, 0, 0, 0, 16256, 32704, 49344, 49152, 32512, 16256, 448, 49600, 65408, 32512, 0}, //  115 s
  {0, 24576, 24576, 24576, 24576, 65024, 65024, 24576, 24576, 24576, 24576, 26112, 32256, 15360, 6144, 0}, //  116 t
  {0, 0, 0, 0, 0, 49344, 49344, 49344, 49344, 49344, 49344, 49344, 57792, 32640, 16128, 0}, //  117 u
  {0, 0, 0, 0, 0, 49344, 49344, 49344, 49344, 49344, 57792, 29568, 16128, 7680, 3072, 0}, //  118 v
  {0, 0, 0, 0, 0, 49344, 49344, 49344, 49344, 52416, 52416, 52416, 65472, 32640, 13056, 0}, //  119 w
  {0, 0, 0, 0, 0, 49344, 57792, 29568, 16128, 7680, 7680, 16128, 29568, 57792, 49344, 0}, //  120 x
  {0, 0, 0, 0, 0, 49344, 49344, 49344, 49344, 57536, 32704, 16320, 192, 448, 16256, 16128}, //  121 y
  {0, 0, 0, 0, 0, 65472, 65472, 896, 1792, 3584, 7168, 14336, 28672, 65472, 65472, 0}, //  122 z
  {0, 0, 0, 6144, 8192, 8192, 8192, 57344, 57344, 8192, 8192, 8192, 6144, 0, 0, 0}, //  123  {
  {0, 0, 0, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 0, 0, 0}, //  124  |
  {0, 0, 0, 49152, 8192, 8192, 8192, 14336, 14336, 8192, 8192, 8192, 49152, 0, 0, 0}, //  125  }
};

unsigned int char_data2[78][12] = {
  {0, 0, 0, 64512, 52224, 19456, 52224, 52224, 52224, 52224, 0, 0}, // 161 ก
  {0, 0, 0, 60416, 60416, 27648, 27648, 27648, 27648, 30720, 0, 0}, // 162 ข
  {0, 0, 0, 44032, 60416, 27648, 27648, 27648, 27648, 30720, 0, 0}, // 163 ฃ
  {0, 0, 0, 30720, 52224, 52224, 60416, 52224, 52224, 52224, 0, 0}, // 164 ค
  {0, 0, 0, 52224, 64512, 52224, 60416, 52224, 52224, 52224, 0, 0}, // 165 ฅ
  {0, 0, 0, 42496, 58880, 26112, 26112, 26112, 30208, 27648, 0, 0}, // 166 ฆ
  {0, 0, 0, 7168, 3072, 52224, 19456, 27648, 11264, 15360, 0, 0}, // 167 ง
  {0, 0, 0, 31744, 50688, 1536, 7680, 1536, 1536, 1536, 0, 0}, // 168 จ
  {0, 0, 0, 30720, 52224, 3072, 52224, 52224, 56320, 27648, 0, 0}, // 169 ฉ
  {0, 0, 0, 58880, 27648, 26112, 26112, 26112, 26112, 31744, 0, 0}, // 170 ช
  {0, 0, 0, 42496, 60416, 26112, 26112, 26112, 26112, 31744, 0, 0}, // 171 ซ
  {0, 0, 0, 63680, 52416, 19648, 52416, 52416, 52928, 60800, 0, 0}, // 172 ฌ
  {0, 0, 0, 63680, 52416, 19648, 52416, 53184, 53184, 57344, 3136, 3968}, // 173 ญ
  {0, 0, 0, 31744, 26112, 9728, 26112, 26112, 26112, 58880, 1536, 32256}, // 174 ฎ
  {0, 0, 0, 31744, 26112, 9728, 26112, 26112, 26112, 58880, 7680, 28160}, // 175 ฏ
  {0, 0, 1536, 32256, 50688, 1536, 7680, 1536, 1536, 1536, 0, 32256}, // 176 ฐ
  {0, 0, 0, 48128, 62976, 26112, 26112, 26112, 26112, 26112, 0, 0}, // 177 ฑ
  {0, 0, 0, 52416, 64704, 52416, 52416, 52416, 61120, 28032, 0, 0}, // 178 ฒ
  {0, 0, 0, 63680, 52416, 19648, 52416, 52416, 60864, 59072, 0, 0}, // 179 ณ
  {0, 0, 0, 31744, 50688, 50688, 62976, 54784, 62976, 26112, 0, 0}, // 180 ด
  {0, 0, 0, 27648, 65024, 50688, 62976, 54784, 62976, 26112, 0, 0}, // 181 ต
  {0, 0, 0, 63488, 52224, 19456, 52224, 52224, 60416, 60416, 0, 0}, // 182 ถ
  {0, 0, 0, 60416, 30208, 26112, 26112, 26112, 26112, 26112, 0, 0}, // 183 ท
  {0, 0, 0, 31744, 49152, 64512, 3072, 52224, 52224, 63488, 0, 0}, // 184 ธ
  {0, 0, 0, 58880, 26112, 26112, 26112, 26112, 28160, 13824, 0, 0}, // 185 น
  {0, 0, 0, 58880, 26112, 26112, 26112, 26112, 26112, 31744, 0, 0}, // 186 บ
  {0, 1536, 1536, 58880, 26112, 26112, 26112, 26112, 26112, 31744, 0, 0}, // 187 ป
  {0, 0, 0, 25344, 49920, 56064, 56064, 56064, 56064, 65024, 0, 0}, // 188 ผ
  {0, 768, 768, 25344, 49920, 56064, 56064, 56064, 56064, 65024, 0, 0}, // 189 ฝ
  {0, 0, 0, 60800, 28032, 28032, 28032, 28032, 28032, 32512, 0, 0}, // 190 พ
  {0, 384, 384, 60800, 28032, 28032, 28032, 28032, 28032, 32512, 0, 0}, // 191 ฟ
  {0, 0, 0, 31744, 26112, 9728, 26112, 26112, 26112, 58880, 0, 0}, // 192 ภ
  {0, 0, 0, 58880, 26112, 26112, 26112, 26112, 30208, 27648, 0, 0}, // 193 ม
  {0, 0, 0, 27648, 52224, 60416, 27648, 52224, 52224, 63488, 0, 0}, // 194 ย
  {0, 0, 0, 30720, 52224, 49152, 63488, 3072, 3072, 6144, 0, 0}, // 195 ร
  {0, 0, 0, 63488, 52224, 19456, 52224, 52224, 60416, 60416, 3072, 3072}, // 196 ฤ
  {0, 0, 0, 30720, 52224, 3072, 31744, 52224, 60416, 60416, 0, 0}, // 197 ล
  {0, 0, 0, 31744, 26112, 9728, 26112, 26112, 26112, 58880, 1536, 1536}, // 198 ฦ
  {0, 0, 0, 28672, 55296, 55296, 6144, 6144, 55296, 28672, 0, 0}, // 199 ว
  {0, 0, 3072, 31744, 52224, 52224, 60416, 52224, 52224, 52224, 0, 0}, // 200 ศ
  {0, 0, 0, 58880, 26112, 26112, 28416, 26112, 26112, 31744, 0, 0}, // 201 ษ
  {0, 0, 3072, 31744, 52224, 3072, 31744, 52224, 60416, 60416, 0, 0}, // 202 ส
  {0, 0, 0, 60928, 28160, 30208, 26112, 26112, 26112, 26112, 0, 0}, // 203 ห
  {0, 0, 192, 60800, 28032, 28032, 28032, 28032, 28032, 32512, 0, 0}, // 204 ฬ
  {0, 0, 0, 30720, 52224, 3072, 60416, 52224, 52224, 64512, 0, 0}, // 205 อ
  {0, 0, 3072, 31744, 52224, 3072, 60416, 52224, 52224, 64512, 0, 0}, // 206 ฮ
  {0, 0, 0, 56320, 64512, 3072, 3072, 3072, 3072, 3072, 0, 0}, // 207 ฯ
  {0, 0, 0, 0, 49152, 61440, 0, 49152, 61440, 0, 0, 0}, // 208 ะ
  {0, 0, 49152, 63488, 0, 0, 0, 0, 0, 0, 0, 0}, // 209 ั
  {0, 0, 0, 57344, 12288, 12288, 12288, 12288, 12288, 12288, 0, 0}, // 210 า
  {49152, 49152, 0, 3584, 768, 768, 768, 768, 768, 768, 0, 0}, // 211 ำ
  {0, 0, 0, 64512, 0, 0, 0, 0, 0, 0, 0, 0}, // 212 ิ
  {0, 0, 3072, 64512, 0, 0, 0, 0, 0, 0, 0, 0}, // 213 ี
  {0, 7168, 5120, 63488, 0, 0, 0, 0, 0, 0, 0, 0}, // 214 ึ
  {0, 5120, 5120, 64512, 0, 0, 0, 0, 0, 0, 0, 0}, // 215 ื
  {0, 57344, 24576, 24576, 0, 0, 0, 0, 0, 0, 0, 0}, // 216 ุ
  {0, 60416, 27648, 31744, 0, 0, 0, 0, 0, 0, 0, 0}, // 217 ู
  {0, 32768, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 218 ฺ
  {8160, 16368, 25080, 57852, 65532, 65532, 65532, 65532, 65532, 32760, 16376, 8167}, //
  {8160, 16368, 25080, 65532, 2044, 124, 124, 2044, 65532, 32761, 16382, 8160}, //
  {},
  {},
  {},
  {0, 0, 0, 49152, 49152, 49152, 49152, 49152, 49152, 49152, 0, 0}, // 224 เ
  {0, 0, 0, 27648, 27648, 27648, 27648, 27648, 27648, 27648, 0, 0}, // 225 แ
  {28672, 57344, 24576, 24576, 24576, 24576, 24576, 24576, 24576, 24576, 0, 0}, // 226 โ
  {24576, 61440, 45056, 12288, 12288, 12288, 12288, 12288, 12288, 12288, 0, 0}, // 227 ใ
  {40960, 61440, 28672, 12288, 12288, 12288, 12288, 12288, 12288, 12288, 0, 0}, // 228 ไ
  {0, 0, 0, 57344, 12288, 12288, 12288, 12288, 12288, 12288, 12288, 12288}, // 229 ๅ
  {0, 0, 0, 55296, 64512, 35840, 52224, 3072, 3072, 3072, 3072, 3072}, // 230 ๆ
  {1024, 30720, 50176, 64512, 0, 0, 0, 0, 0, 0, 0, 0}, // 231 ็
  {12288, 12288, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 232 ่
  {13312, 6144, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 233 ้
  {29696, 22528, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 234 ๊
  {6144, 15360, 6144, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 235 ๋
  {3072, 6144, 7168, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 236 ์
  {3072, 3072, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 237 ํ
  {3072, 3072, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 238 ๎
};

void TD_setPoint(uint16_t r, uint16_t c, bool state) {
  uint16_t nr = 0;
  uint16_t nc = 0;
  uint16_t n = 0;
  uint16_t c0 = 0;

  if (r <= 0 || r > TD_max_row) return;
  if (c <= 0 || c > TD_max_col) return;

  n = 0;
  if (r > 8) {
    r = r - 8;
    n = 32;
  }

  c0 = c;
  nc = 1;
  while (c0 > 32) {
    c0 = c0 - 32;
    nc = nc + 1;
  }

  nc = (TD_max_col * 2) - (32 * nc) - c + n;
  nr = r - 1;

  mx.MysetPoint(nr, nc, state);
}

char * TD_IntToBin(unsigned int p_ui) {
  static char ret[17];        // need to change if expand font width

  for (int i = 0; i < TD_max_char_col; i++) ret[TD_max_char_col - 1 - i] = '0' + ((p_ui & (1 << i)) > 0);
  ret[TD_max_char_col] = '\0';

  return ret;
}

void TD_SerialShowBinChar2(int p_idx) {
  String c;

  Serial.println(p_idx);
  for (int i = 0; i < TD_max_char_row2; i++) {
    c = TD_IntToBin(char_data2[p_idx][i]);
    c.replace("1", "X");
    c.replace("0", " ");
    Serial.println(c);
  }
  Serial.println("");
}

int TD_IsX1(int idx) {
  int ret = 0;

  for (int i = 0; i < char_char_x1; i++) {
    if (char_x1[i] == idx) {
      ret = 1;
      break;
    }
  }

  return ret;
}

int TD_IsX2(int idx) {
  int ret = 0;

  for (int i = 0; i < char_char_x2; i++) {
    if (char_x2[i] == idx) {
      ret = 1;
      break;
    }
  }

  return ret;
}

void TD_WriteChar(int p_r, int p_c, int p_idx) {
  char * c;

  if (p_idx == 219) {
    if (g_pcman_n == 0) {
      if (g_pcman_char == 220) {
        g_pcman_char = 219;
      } else {
        g_pcman_char = 220;
      }
    }
    p_idx = g_pcman_char;
    g_pcman_n = g_pcman_n + 1;
    if (g_pcman_n == 8) g_pcman_n = 0;
  }

  // not thai
  if (p_idx < TD_start_thai_char) {
    p_idx = p_idx - TD_start_not_thai_char;
    //TD_SerialShowBinChar2(p_idx);

    for (int i = 0; i < TD_max_char_row1; i++) {
      c =  TD_IntToBin(char_data1[p_idx][i]);
      for (int j = 0; j < TD_max_char_col; j++)
        if (c[j] == '1') {
          TD_setPoint(p_r + i, p_c + j, true);
        }
    }
    // thai
  } else {
    p_idx = p_idx - TD_start_thai_char;
    //TD_SerialShowBinChar2(p_idx);

    for (int i = 0; i < TD_max_char_row2; i++) {
      c =  TD_IntToBin(char_data2[p_idx][i]);
      for (int j = 0; j < TD_max_char_col; j++)
        if (c[j] == '1') {
          TD_setPoint(p_r + i, p_c + j, true);
        }
    }
  }
}

int TD_CharWidth(int p_idx) {
  char * c;
  int w = 0;

  if (p_idx < TD_start_thai_char) {
    p_idx = p_idx - TD_start_not_thai_char;

    for (int i = 0; i < TD_max_char_row1; i++) {
      c =  TD_IntToBin(char_data1[p_idx][i]);
      for (int j = 0; j < TD_max_char_col; j++)
        if (c[j] == '1') {
          if (j >= w) w = j;
        }
    }

    // thai
  } else {
    p_idx = p_idx - TD_start_thai_char;

    for (int i = 0; i < TD_max_char_row2; i++) {
      c =  TD_IntToBin(char_data2[p_idx][i]);
      for (int j = 0; j < TD_max_char_col; j++)
        if (c[j] == '1') {
          if (j >= w) w = j;
        }
    }
  }

  return w + 1;
}

int TD_Char2Width(int p_idx) {
  char * c;
  int w = 0;

  for (int i = 0; i < TD_max_char_row2; i++) {
    c =  TD_IntToBin(char_data2[p_idx][i]);
    for (int j = 0; j < TD_max_char_col; j++)
      if (c[j] == '1') {
        if (j >= w) w = j;
      }
  }

  return w + 1;
}

void TD_LEDWriteText(int p_r, int p_c, String p_text) {
  char c1;
  char c2;
  int w = 0;
  int idx = 0;
  int normal;
  String myText;

  myText = p_text;
  mx.clear();
  for (int i = 0; i < myText.length(); i++) {
    c1 = myText[i];
    // E0 224 Thai
    if ((int)c1 == 224) {
      c1 = myText[i + 1];
      c2 = myText[i + 2];
      // B8,81(129)  ก - ฮ
      if ((int)c1 == 184 && ((int)c2 + 32) >= 161 && ((int)c2 + 32) <= 207) {
        // ก 81 = 129 --> +32 --> 161
        idx = (int)c2 + 32;
      }
      // B8,B0(176)  สระ
      if ((int)c1 == 184 && ((int)c2 + 32) >= 208 && ((int)c2 + 32) <= 218) {
        // ะ B0 = 176 --> +32 --> 208
        idx = (int)c2 + 32;
      }
      // B9,80(128)  สระ
      if ((int)c1 == 185 && ((int)c2 + 96) >= 224 && ((int)c2 + 96) <= 231) {
        // เ 80 = 176 --> +96 --> 224
        idx = (int)c2 + 96;
      }
      // B9,B0(136)  วรรณยุก
      if ((int)c1 == 185 && ((int)c2 + 96) >= 232 && ((int)c2 + 96) <= 238) {
        // ่ 88 = 136 --> +96 --> 208
        idx = (int)c2 + 96;
      }
      i = i + 2;
    } else {
      idx = (int)c1;
    }

    normal = 1;
    // thai over
    if (TD_IsX1(idx) == 1) {
      w = TD_CharWidth(idx);
      TD_WriteChar(p_r - 2, p_c - (w + TD_gap_pixel), idx);
      normal = 0;
    }
    // thai under
    if (TD_IsX2(idx) == 1) {
      w = TD_CharWidth(idx);
      TD_WriteChar(p_r + 10, p_c - (w + TD_gap_pixel), idx);
      normal = 0;
    }
    if (normal == 1) {
      switch (idx) {
        case 32:  // space
          p_c = p_c + 8;
          break;
        case 211: // ำ
          w = TD_CharWidth(idx);
          TD_WriteChar(p_r, p_c - (3 + TD_gap_pixel), idx);
          p_c = p_c - 3 + w + TD_gap_pixel;
          break;
        default:
          TD_WriteChar(p_r, p_c, idx);
          w = TD_CharWidth(idx);
          p_c = p_c + w + TD_gap_pixel;
          break;
      }
    }
  }
  mx.MyflushBufferAll();
  delay(TD_led_delay);

}

int TD_LEDTextPixel(String p_text) {
  char c1;
  char c2;
  int w = 0;
  int idx = 0;
  int normal;
  String myText;
  int p_c;

  p_c = 0;
  myText = p_text;
  for (int i = 0; i < myText.length(); i++) {
    c1 = myText[i];
    // E0 224 Thai
    if ((int)c1 == 224) {
      c1 = myText[i + 1];
      c2 = myText[i + 2];
      // B8,81(129)  ก - ฮ
      if ((int)c1 == 184 && ((int)c2 + 32) >= 161 && ((int)c2 + 32) <= 207) {
        // ก 81 = 129 --> +32 --> 161
        idx = (int)c2 + 32;
      }
      // B8,B0(176)  สระ
      if ((int)c1 == 184 && ((int)c2 + 32) >= 208 && ((int)c2 + 32) <= 218) {
        // ะ B0 = 176 --> +32 --> 208
        idx = (int)c2 + 32;
      }
      // B9,80(128)  สระ
      if ((int)c1 == 185 && ((int)c2 + 96) >= 224 && ((int)c2 + 96) <= 231) {
        // เ 80 = 176 --> +96 --> 224
        idx = (int)c2 + 96;
      }
      // B9,B0(136)  วรรณยุก
      if ((int)c1 == 185 && ((int)c2 + 96) >= 232 && ((int)c2 + 96) <= 238) {
        // ่ 88 = 136 --> +96 --> 208
        idx = (int)c2 + 96;
      }
      i = i + 2;
    } else {
      idx = (int)c1;
    }

    normal = 1;
    // thai over
    if (TD_IsX1(idx) == 1) {
    }
    // thai under
    if (TD_IsX2(idx) == 1) {
      w = TD_CharWidth(idx);
      normal = 0;
    }
    if (normal == 1) {
      switch (idx) {
        case 32:  // space
          p_c = p_c + 8;
          break;
        case 211: // ำ
          w = TD_CharWidth(idx);
          p_c = p_c - 3 + w + TD_gap_pixel;
          break;
        default:
          w = TD_CharWidth(idx);
          p_c = p_c + w + TD_gap_pixel;
          break;
      }
    }
  }

  return p_c;

}

void TD_LEDText(String p_text) {
  TD_LEDWriteText(TD_normal_row, 1, p_text);
}

void TD_LEDScrollText(String p_text) {
  int c;
  int n;

  n = TD_LEDTextPixel(p_text);
  for (int i = TD_max_col; i >= n * -1; i--) {
    TD_LEDWriteText(TD_normal_row, i, p_text);
  }
}

//////////////////////////////////////////*******************************************************************************************/////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  mx.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
  Serial.println("Waiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  updateData();
}

void loop() {
  curTime = millis();
  dots = (curTime % 1000) < 500;
  time_t now = time(nullptr);
  struct tm* newtime = localtime(&now);
  tmy = (newtime->tm_year + 2443);
  tmm = (newtime->tm_mon);
  tmw = (newtime->tm_wday);
  tmd = (newtime->tm_mday);
  tms = "";
  if ((newtime->tm_hour) > 9 || (newtime->tm_hour) < 20 ) tms += " ";
  if ((newtime->tm_hour) < 10) tms += "0";
  tms += (newtime->tm_hour);
  if (dots) tms += ":"; else tms += " ";
  if ((newtime->tm_min) < 10) tms += "0";
  tms += (newtime->tm_min);

  drawTime();
  
  if (millis() - lastDownloadUpdate > 1000 * UPDATE_INTERVAL_SECS) {
    updateData();
    lastDownloadUpdate = millis();
  }
  
  if (curTime - changeMode >= 30000) drawWeather();
  
}

void drawTime() {
  TD_LEDText(tms);
}

void updateData() {
  TD_LEDScrollText((String)"อัพเดท");
  WundergroundConditions *conditionsClient = new WundergroundConditions(IS_METRIC);
  conditionsClient->updateConditions(&conditions, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  delete conditionsClient;
  conditionsClient = nullptr;
}

void drawWeather() {
  changeMode = millis();
  String temp = conditions.currentTemp;
  
  String condi = "";
  if (conditions.weatherText == "Chance of Flurries") condi += "อาจจะมีละอองฝน";
  if (conditions.weatherText == "Chance of Rain") condi += "อาจจะมีฝนตก";
  if (conditions.weatherText == "Chance of Sleet") condi += "อาจจะมีฝนหิมะ";
  if (conditions.weatherText == "Chance of Snow") condi += "อาจจะมีหิมะตก";
  if (conditions.weatherText == "Chance of Storms") condi += "อาจจะมีพายุ";
  if (conditions.weatherText == "Clear") condi += "บรรยากาศแจ่มใส";
  if (conditions.weatherText == "Cloudy") condi += "มีเมฆมาก";
  if (conditions.weatherText == "Flurries") condi += "มีฝนตกปรอยๆ";
  if (conditions.weatherText == "Fog") condi += "มีหมอกบาง";
  if (conditions.weatherText == "Hazy") condi += "มีหมอกหนา";
  if (conditions.weatherText == "Mostly Cloudy") condi += "มีเมฆเป็นส่วนใหญ่";
  if (conditions.weatherText == "Mostly Sunny") condi += "มีแดดออกเป็นส่วนใหญ่";
  if (conditions.weatherText == "Partly Cloudy") condi += "มีเมฆเป็นบางส่วน";
  if (conditions.weatherText == "Partly Sunny") condi += "มีแดดออกเป็นบางส่วน";
  if (conditions.weatherText == "Sleet") condi += "มีฝนหิมะ";
  if (conditions.weatherText == "Rain") condi += "มีฝนตก";
  if (conditions.weatherText == "Snow") condi += "มีหิมะตก";
  if (conditions.weatherText == "Sunny") condi += "มีแดดจ้า";
  if (conditions.weatherText == "Storms") condi += "มีพายุ";

  String tmwtemp = tmw[weekdayNames];
  String tmmtemp = tmm[monthNames];

  TD_LEDScrollText(tmwtemp + " " + tmd + " " + tmmtemp + " " + tmy + " อุณหภูมิ " + temp + "C " + condi);

}
