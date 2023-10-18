#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <regex>

#define DwinSerial Serial2
#define pinAnalog 27   // Check lại Pin
#define pinOpenGate 26 // Check lại Pin
#define PinPresure 37  // check lại pin

#define LoraM0 32 // Check lại Pin
#define LoraM1 33 // Check lại Pin

const char *ssid = "TENSIOMETERNG";
const char *password = "12345678";
const int udpPort = 12345;

WiFiUDP udp;
char packetBuffer[255];

const char *ssidAP = "Presure_Sensor";
const char *passAP = "12345678";

static float positionSensor1 = 0.0;
static float positionSensor2 = 0.0;
static float positionSensor3 = 0.0;
static float positionSensor4 = 0.0;
static float positionSensor5 = 0.0;
static float positionSensor6 = 0.0;
static int vBat;
int id;
float valuePressure = 0.0;
//************************************************************************************************
void ShowBatDisplay();

void DGUS_SendVal(int iAdr, int iVal)
{
  // static unsigned long timeSend=millis();
  // if(millis()-timeSend>100){
  byte bAdrL, bAdrH, bValL, bValH;
  bAdrL = iAdr & 0xFF;
  bAdrH = (iAdr >> 8) & 0xFF;
  bValL = iVal & 0xFF;
  bValH = (iVal >> 8) & 0xFF;
  DwinSerial.write(0x5A);
  DwinSerial.write(0xA5);
  DwinSerial.write(0x05);
  DwinSerial.write(0x82);
  DwinSerial.write(bAdrH);
  DwinSerial.write(bAdrL);
  DwinSerial.write(bValH);
  DwinSerial.write(bValL);
  // timeSend=millis();
  // }
}

void parseString(const String &input, int &order, float &value1, int &value2)
{
  int commaPos = input.indexOf(",");

  if (commaPos != -1)
  {
    order = input.substring(0, commaPos).toInt();
    int colonPos = input.indexOf(":");

    if (colonPos != -1)
    {
      String values = input.substring(colonPos + 1);
      values.trim();
      int secondCommaPos = values.indexOf(",");

      if (secondCommaPos != -1)
      {
        String value1Str = values.substring(0, secondCommaPos);
        value1 = value1Str.toFloat();

        String value2Str = values.substring(secondCommaPos + 1);
        value2 = value2Str.toInt();
      }
    }
  }
}

void FloatToHex(float f, byte *hex)
{
  byte *f_byte = reinterpret_cast<byte *>(&f);
  memcpy(hex, f_byte, 4);
}
// Giá trị hiển thị lên DWIN float 4 Byte
void sendFloatNumber(float floatValue, long VPAddress)
{
  byte bAdrL, bAdrH, bValL, bValH;
  bAdrL = VPAddress & 0xFF;
  bAdrH = (VPAddress >> 8) & 0xFF;

  DwinSerial.write(0x5A);
  DwinSerial.write(0xA5);
  DwinSerial.write(0x07);
  DwinSerial.write(0x82);
  DwinSerial.write(bAdrH);
  DwinSerial.write(bAdrL);

  byte hex[4] = {0}; // create a hex array for the 4 bytes

  FloatToHex(floatValue, hex);
  DwinSerial.write(hex[3]);
  DwinSerial.write(hex[2]);
  DwinSerial.write(hex[1]);
  DwinSerial.write(hex[0]);
}
//************************************************************************************************
void setup()
{
  Serial.begin(9600);
  Serial2.begin(115200);

  pinMode(pinOpenGate, OUTPUT);
  pinMode(LoraM0, OUTPUT);
  pinMode(LoraM1, OUTPUT);

  WiFi.softAP(ssid, password);

  IPAddress localIP = WiFi.softAPIP();
  Serial.print("Master IP address: ");
  Serial.println(localIP);
  udp.begin(udpPort);
}
//----------------------------------------------------------------

void loop()
{
  int packetSize = udp.parsePacket();
  if (packetSize)
  {
    int len = udp.read(packetBuffer, 255);
    if (len > 0)
    {
      packetBuffer[len] = 0;
      Serial.printf("Received packet: %s\n", packetBuffer);

      // Chuyển đổi packetBuffer thành một đối tượng chuỗi std::string
      std::string packetString(packetBuffer);

      // Sử dụng biểu thức chính quy để trích xuất giá trị áp suất
      std::regex pattern("\\+?-?\\d+\\.\\d+"); // Biểu thức chính quy cho số thực có hoặc không có dấu '+' hoặc '-'

      std::smatch match;
      if (std::regex_search(packetString, match, pattern))
      {
        std::string pressureValue = match.str();
        Serial.printf("Pressure Value: %s\n", pressureValue.c_str());
        float float_value = atof(pressureValue.c_str()); // Chuyển đổi chuỗi thành float
        sendFloatNumber(float_value, 0x1001);
      }
      else
      {
        Serial.println("Failed to extract pressure value.");
      }
    }
  }
}
