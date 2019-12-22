#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>  
#include <TinyGPS++.h>
// Cấu hình.
#define FIREBASE_HOST "https://busmap-4731d.firebaseio.com"
#define FIREBASE_AUTH "62bgmV3TLO7Fk86Xe7goyxL5SM8JylG5MvhnQfu0"
#define WIFI_SSID "MThien 2.4hz"
#define WIFI_PASSWORD "123456787"

//Define Firebase Data object
FirebaseData firebaseData;

unsigned long sendDataPrevMillis = 0;

void printResult(FirebaseData &data);

static const int RXPin = 12, TXPin = 13; //RX->D7 and TX->D6
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin); 

void setup()
{

    Serial.begin(115200);
    Serial.println();
    Serial.println();
  
    ss.begin(GPSBaud);  
  
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);

   
}

void loop()
{
    while (ss.available() > 0) //while data is available
    if (gps.encode(ss.read())) //read gps data
    {
        if (millis() - sendDataPrevMillis > 15000)
          {
          sendDataPrevMillis = millis();
          
          String path = "/02499988";
      
          String jsonStr = "";
      
          FirebaseJson json1;
      
          FirebaseJsonData jsonObj;

          json1.set("id", "024");
          json1.set("idBusInfo", "99988");
          json1.set("angle", 50);
          json1.set("Latitude", gps.location.lat());
          json1.set("Longitude", gps.location.lng());
          
      
          Serial.println("------------------------------------");
          Serial.println("JSON Data");
          json1.toString(jsonStr, true);
          Serial.println(jsonStr);
          Serial.println("------------------------------------");
      
          Serial.println("------------------------------------");
          Serial.println("Set JSON test...");
      
          if (Firebase.set(firebaseData, path, json1))
          {
            Serial.println("PASSED");
            Serial.println("PATH: " + firebaseData.dataPath());
            Serial.println("TYPE: " + firebaseData.dataType());
            Serial.print("VALUE: ");
            printResult(firebaseData);
            Serial.println("------------------------------------");
            Serial.println();
          }
          else
          {
            Serial.println("FAILED");
            Serial.println("REASON: " + firebaseData.errorReason());
            Serial.println("------------------------------------");
            Serial.println();
          }
      
          json1.setJsonData(jsonStr);
          json1.get(jsonObj, "Bus");
          Serial.println("Bus: " + jsonObj.stringValue);
      
      
          Serial.println("------------------------------------");
          Serial.println();
          }
     
    }
}

void printResult(FirebaseData &data)
{

    if (data.dataType() == "int")
        Serial.println(data.intData());
    else if (data.dataType() == "float")
        Serial.println(data.floatData(), 5);
    else if (data.dataType() == "double")
        printf("%.9lf\n", data.doubleData());
    else if (data.dataType() == "boolean")
        Serial.println(data.boolData() == 1 ? "true" : "false");
    else if (data.dataType() == "string")
        Serial.println(data.stringData());
    else if (data.dataType() == "json")
    {
        Serial.println();
        FirebaseJson &json = data.jsonObject();
        //Print all object data
        Serial.println("Pretty printed JSON data:");
        String jsonStr;
        json.toString(jsonStr, true);
        Serial.println(jsonStr);
        Serial.println();
        Serial.println("Iterate JSON data:");
        Serial.println();
        size_t len = json.iteratorBegin();
        String key, value = "";
        int type = 0;
        for (size_t i = 0; i < len; i++)
        {
            json.iteratorGet(i, type, key, value);
            Serial.print(i);
            Serial.print(", ");
            Serial.print("Type: ");
            Serial.print(type == JSON_OBJECT ? "object" : "array");
            if (type == JSON_OBJECT)
            {
                Serial.print(", Key: ");
                Serial.print(key);
            }
            Serial.print(", Value: ");
            Serial.println(value);
        }
        json.iteratorEnd();
    }
    else if (data.dataType() == "array")
    {
        Serial.println();
        //get array data from FirebaseData using FirebaseJsonArray object
        FirebaseJsonArray &arr = data.jsonArray();
        //Print all array values
        Serial.println("Pretty printed Array:");
        String arrStr;
        arr.toString(arrStr, true);
        Serial.println(arrStr);
        Serial.println();
        Serial.println("Iterate array values:");
        Serial.println();
        for (size_t i = 0; i < arr.size(); i++)
        {
            Serial.print(i);
            Serial.print(", Value: ");

            FirebaseJsonData &jsonData = data.jsonData();
            //Get the result data from FirebaseJsonArray object
            arr.get(jsonData, i);
            if (jsonData.typeNum == JSON_BOOL)
                Serial.println(jsonData.boolValue ? "true" : "false");
            else if (jsonData.typeNum == JSON_INT)
                Serial.println(jsonData.intValue);
            else if (jsonData.typeNum == JSON_DOUBLE)
                printf("%.9lf\n", jsonData.doubleValue);
            else if (jsonData.typeNum == JSON_STRING ||
                     jsonData.typeNum == JSON_NULL ||
                     jsonData.typeNum == JSON_OBJECT ||
                     jsonData.typeNum == JSON_ARRAY)
                Serial.println(jsonData.stringValue);
        }
    }
}
