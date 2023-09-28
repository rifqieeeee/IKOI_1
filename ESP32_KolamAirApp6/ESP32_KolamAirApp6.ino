#include <WiFi.h>
#include <PubSubClient.h>
#include "MAX6675.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>

//#1 Sensor Ultrasonic
const int trigPin = 13;
const int echoPin = 12;
//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
long duration;
float distanceCm;
float distanceInch;

//#2 Sensor Suhu
// note: pins are slightly different than other examples!
const int dataPin   = 19;
const int clockPin  = 5;
const int selectPin = 23;
MAX6675 thermoCouple;
uint32_t start, stop;
float temper;

//#3 Sensor PH
float calibration_value = 20.24 - 0.7; //21.34 - 0.7
int phval = 0; 
unsigned long int avgval;
int buffer_arr[10],temp;
int phpin = 34;
float ph_act;

//#4 Relay
const int buttonPin = 26;
int buttonState = 0;
int feeder = 15;
int lamp = 2;
int waterbackwash = 4;
int waterpump = 16;
int aerator = 3;
int flag = 0;
int led_out, led_time;
int led_out2, led_time2;
int led_out3, led_time3;
int led_out4, led_time4;
int led_out5, led_time5;

//#5 WIFI
const char *ssid = "Sekolah Robot Indonesia";
const char *password = "sadapsadap";
int status_wifi = 0;
float param;

//#6 MQTT
const char *mqtt_broker = "broker.emqx.io";
const char *topic1 = "ikoi/1/1/sensor/temperature";
const char *topic2 = "ikoi/1/1/sensor/waterdepth";
const char *topic3 = "ikoi/1/1/sensor/ph";
const char *topic_out1 = "ikoi/1/1/aktuator/lamp/command";
const char *topic_out2 = "ikoi/1/1/aktuator/waterpump/command";
const char *topic_out3 = "ikoi/1/1/aktuator/waterbackwash/command";
const char *topic_out4 = "ikoi/1/1/aktuator/feeder/command";
const char *topic_out5 = "ikoi/1/1/aktuator/aerator/command";
const char *topic_out6 = "ikoi/1/1/auto/pond";
const char *topic_out7 = "ikoi/1/1/auto/feeder";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

//#7 Waktu
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String dayStamp;
String timeStamp;

//#8 feeder
int flag_feeder = 0;

//#9 Kolam
int dalam_kolam = 200;
int auto_pond = 1;
int auto_feed = 1;
int batas_isi = 150;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //#1
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  //#2
  thermoCouple.begin(clockPin, selectPin, dataPin);
  thermoCouple.setSPIspeed(4000000);

  //#4
  pinMode(lamp, OUTPUT);
  pinMode(waterpump, OUTPUT);
  pinMode(waterbackwash, OUTPUT);
  pinMode(feeder, OUTPUT);
  pinMode(aerator, OUTPUT);

  //#5
  WiFi.begin(ssid, password);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    i += 1;
    delay(500);
    Serial.print(i);
    if (i > 30){
      if(status_wifi == 1){
        WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP    
        WiFiManager wm;
        wm.resetSettings();
        bool res;
        res = wm.autoConnect("IKOI_Home ","password"); // password protected ap
        if(!res) {
            Serial.println("Failed to connect");
        } 
        else {   
            Serial.println("Connected to the WiFi network");
        }
      }status_wifi = 1;
    }
  }
  Serial.println("Connected to the WiFi network");

  //#7
  timeClient.begin();
  timeClient.setTimeOffset(25200);

  //#6
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp8266-client-";
    client_id += String(WiFi.macAddress());
    Serial.println("Connecting to public emqx mqtt broker.....");
    while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Public emqx mqtt broker connected");
      } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
      }        
    }
  }
  // publish and subscribe
  client.publish(topic_out1, "hello emqx");
  client.subscribe(topic_out1);
  client.subscribe(topic_out2);
  client.subscribe(topic_out3);
  client.subscribe(topic_out4);
  client.subscribe(topic_out5);
}

void callback(char *topic, byte *payload, unsigned int length) {
//  Serial.print("Message arrived in topic: ");
//  Serial.println(topic);
//  Serial.print("Message:");
  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char) payload[i];  // convert *byte to string
  }
//  Serial.print(message);
//  Serial.println();
//  Serial.println("-----------------------");

    if (String(topic) == "ikoi/1/1/aktuator/lamp/command"){
      String  str;
      String strs[20];
      int StringCount = 0;
      str = message;
      Serial.println(str);
      while (str.length() > 0){
        int index = str.indexOf('/');
        if (index == -1){
          strs[StringCount++] = str;
        }
        else{
          strs[StringCount++] = str.substring(0, index);
          str = str.substring(index+1);
        }
      }
      Serial.println(strs[0]);
      led_out = strs[0].toInt();
      Serial.println(strs[1]);
      led_time = strs[1].toInt();
    }

    if (String(topic) == "ikoi/1/1/aktuator/waterpump/command"){
      String  str;
      String strs[20];
      int StringCount = 0;
      str = message;
      Serial.println(str);
      while (str.length() > 0){
        int index = str.indexOf('/');
        if (index == -1){
          strs[StringCount++] = str;
        }
        else{
          strs[StringCount++] = str.substring(0, index);
          str = str.substring(index+1);
        }
      }
      Serial.println(strs[0]);
      led_out2 = strs[0].toInt();
      Serial.println(strs[1]);
      led_time2 = strs[1].toInt();
    }

    if (String(topic) == "ikoi/1/1/aktuator/waterbackwash/command"){
      String  str;
      String strs[20];
      int StringCount = 0;
      str = message;
      Serial.println(str);
      while (str.length() > 0){
        int index = str.indexOf('/');
        if (index == -1){
          strs[StringCount++] = str;
        }
        else{
          strs[StringCount++] = str.substring(0, index);
          str = str.substring(index+1);
        }
      }
      Serial.println(strs[0]);
      led_out3 = strs[0].toInt();
      Serial.println(strs[1]);
      led_time3 = strs[1].toInt();
    }

    if (String(topic) == "ikoi/1/1/aktuator/feeder/command"){
      String  str;
      String strs[20];
      int StringCount = 0;
      str = message;
      Serial.println(str);
      while (str.length() > 0){
        int index = str.indexOf('/');
        if (index == -1){
          strs[StringCount++] = str;
        }
        else{
          strs[StringCount++] = str.substring(0, index);
          str = str.substring(index+1);
        }
      }
      Serial.println(strs[0]);
      led_out4 = strs[0].toInt();
      Serial.println(strs[1]);
      led_time4 = strs[1].toInt();
    }

    if (String(topic) == "ikoi/1/1/aktuator/aerator/command"){
      String  str;
      String strs[20];
      int StringCount = 0;
      str = message;
      Serial.println(str);
      while (str.length() > 0){
        int index = str.indexOf('/');
        if (index == -1){
          strs[StringCount++] = str;
        }
        else{
          strs[StringCount++] = str.substring(0, index);
          str = str.substring(index+1);
        }
      }
      Serial.println(strs[0]);
      led_out5 = strs[0].toInt();
      Serial.println(strs[1]);
      led_time5 = strs[1].toInt();
    }

    if (String(topic) == "ikoi/1/1/auto/pond"){
      String  str;
      String strs[20];
      int StringCount = 0;
      str = message;
      Serial.println(str);
      if (str == 1)auto_pond = 1;
      if (str == 0)auto_pond = 0;
    }

    if (String(topic) == "ikoi/1/1/auto/feeder"){
      String  str;
      String strs[20];
      int StringCount = 0;
      str = message;
      Serial.println(str);
      if (str == 1)auto_feeder = 1;
      if (str == 0)auto_feeder   = 0;
    }
}

void reconnect() {
  while (!client.connected()) {
    String client_id = "esp8266-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }        
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  read_data();
  get_time();
  buttonState = digitalRead(buttonPin);

//#4
  if(led_out==1){
    digitalWrite(lamp, HIGH);
    led_time-=4;
    Serial.println(led_time);
    if(led_time==0){
      led_out = 0;
      digitalWrite(lamp, LOW);
    }
  }else digitalWrite(lamp, LOW);

  if(led_out2==1){
    digitalWrite(waterpump, HIGH);
    led_time2-=4;
    Serial.println(led_time2);
    if(led_time2==0){
      led_out2 = 0;
      digitalWrite(waterpump, LOW);
    }
  }else digitalWrite(waterpump, LOW);

  if(led_out3==1){
    digitalWrite(waterbackwash, HIGH);
    led_time3-=4;
    Serial.println(led_time);
    if(led_time3==0){
      led_out3 = 0;
      digitalWrite(waterbackwash, LOW);
    }
  }else digitalWrite(waterbackwash, LOW);

  if(led_out4==1){
    digitalWrite(feeder, HIGH);
    led_time4-=4;
    Serial.println(led_time);
    if(led_time4==0){
      led_out4 = 0;
      digitalWrite(feeder, LOW);
    }
  }else digitalWrite(feeder, LOW);

  if(led_out5==1){
    digitalWrite(aerator, HIGH);
    led_time5-=4;
    Serial.println(led_time2);
    if(led_time5==0){
      led_out5 = 0;
      digitalWrite(aerator, LOW);
    }
  }else digitalWrite(aerator, LOW);

  if (flag_feeder > 36000){
    digitalWrite(feeder, HIGH);
    delay(3000);
    digitalWrite(feeder, LOW);
    flag_feeder = 0;
  }
  
//  Serial.print(flag_feeder);
//  Serial.print(", ");
//  Serial.println(flag);
  
  if(flag==250){
    send_data();
    flag++;
  }else if ((flag==500)){
    send_data();
    flag++;
  }else if ((flag==750)){
    send_data();
    flag = 0;
  }else{
    client.loop();
    flag++;
  }
}

void read_data(){
  //#1
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance
  distanceCm = 200.0 - (duration * SOUND_SPEED/2);

  //#2
  start = micros();
  int status = thermoCouple.read();
  stop = micros();
  temper = thermoCouple.getTemperature();

  //#3
  avgval = analogRead(phpin);
  float volt=(float)avgval*3.3/4096.0; 
  ph_act = (-5.70 * volt + calibration_value);

  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.print(distanceCm);
  Serial.print("\ttemp: ");
  Serial.print(temper);
  Serial.print("\tpH Val: ");
  Serial.println(ph_act);
}

void send_data(){
  static char tempTemp[7];            dtostrf(temper, 6, 2, tempTemp);
  static char distanceCmTemp[7];      dtostrf(distanceCm, 6, 2, distanceCmTemp);
  static char ph_actTemp[7];          dtostrf(ph_act, 6, 2, ph_actTemp);

  String sens1 = String(tempTemp) + "/" + String(dayStamp) + "/" + String(timeStamp);
  String sens2 = String(distanceCmTemp) + "/" + String(dayStamp) + "/" + String(timeStamp);
  String sens3 = String(ph_actTemp) + "/" + String(dayStamp) + "/" + String(timeStamp);

  if(flag==250){
    client.publish(topic1, sens1.c_str ());           client.subscribe(topic1);
  }else if ((flag==500)){
    client.publish(topic2, sens2.c_str ());     client.subscribe(topic2);
  }else if ((flag==750)){
    client.publish(topic3, sens3.c_str ());         client.subscribe(topic3);
  }

  if(auto_feed == 1){
    if (temper >= 22)  += 10;
    else if (temper >= 18) flag_feeder += 5;
    else if (temper >= 16) flag_feeder += 2;
    else flag_feeder += 0;
  }

  if(auto_pond == 1){
    if (ph_act < 6.5 || ph_act > 8.0){
      while (distanceCm > batas_isi){
        digitalWrite(waterbackwash, LOW);
      }
      digitalWrite(waterbackwash, HIGH);
      
      while (distanceCm > 10){
        digitalWrite(waterpump, LOW);
      }
      digitalWrite(waterpump, HIGH);
    }
  }

  
}

void get_time(){
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }

  formattedDate = timeClient.getFormattedDate();
  //Serial.println(formattedDate);

  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
//  Serial.print("DATE: ");
//  Serial.println(dayStamp);

  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
//  Serial.print("HOUR: ");
//  Serial.println(timeStamp);
}
