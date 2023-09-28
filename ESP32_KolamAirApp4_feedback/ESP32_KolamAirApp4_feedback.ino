#include <WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>

int lamp = 2; //fix
int feeder = 4; 
int waterpump = 16;
int waterbackwash = 17; //fix
int aerator = 5; //fix

int current_lamp = 0;
int past_lamp = 0;
int current_feeder = 0;
int past_feeder = 0;
int current_waterpump = 0;
int past_waterpump = 0;
int current_waterbackwash = 0;
int past_waterbackwash = 0;
int current_aerator = 0;
int past_aerator = 0;

const char *ssid = "Sekolah Robot Indonesia";
const char *password = "sadapsadap";
int status_wifi = 0;
float param;

const char *mqtt_broker = "broker.emqx.io";
const char *topic_out1 = "ikoi/1/1/aktuator/lamp/feedback";
const char *topic_out2 = "ikoi/1/1/aktuator/waterpump/feedback";
const char *topic_out3 = "ikoi/1/1/aktuator/waterbackwash/feedback";
const char *topic_out4 = "ikoi/1/1/aktuator/feeder/feedback";
const char *topic_out5 = "ikoi/1/1/aktuator/aerator/feedback";
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(lamp, INPUT);
  pinMode(feeder, INPUT);
  pinMode(waterpump, INPUT);
  pinMode(waterbackwash, INPUT);
  pinMode(aerator, INPUT);
//  digitalWrite(13, HIGH);

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
        res = wm.autoConnect("IKOI_Feedback","password"); // password protected ap
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
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char) payload[i];  // convert *byte to string
  }
  Serial.print(message);
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
  client.loop();
  get_time();
  kirim_data();
}

void kirim_data(){
  current_lamp = digitalRead(lamp);
  current_feeder = digitalRead(feeder);
  current_waterpump = digitalRead(waterpump);
  current_waterbackwash = digitalRead(waterbackwash);
  current_aerator = digitalRead(aerator);

  Serial.print(current_lamp);
  Serial.print(",");
  Serial.print(current_feeder);
  Serial.print(",");
  Serial.print(current_waterpump);
  Serial.print(",");
  Serial.print(current_waterbackwash);
  Serial.print(",");
  Serial.println(current_aerator);
  

  if (current_lamp > past_lamp){
    int status_lamp = 1;
    String sens1 = String(status_lamp) + "/" + String(dayStamp) + "/" + String(timeStamp);
    client.publish(topic_out1, sens1.c_str ());           client.subscribe(topic_out1);
    past_lamp = current_lamp;
  } else if (current_lamp < past_lamp){
    int status_lamp = 0;
    String sens1 = String(status_lamp) + "/" + String(dayStamp) + "/" + String(timeStamp);
    client.publish(topic_out1, sens1.c_str ());           client.subscribe(topic_out1);
    past_lamp = current_lamp;
  } else past_lamp = current_lamp;

  if (current_waterpump > past_waterpump){
    int status_waterpump = 1;
    String sens1 = String(status_waterpump) + "/" + String(dayStamp) + "/" + String(timeStamp);
    client.publish(topic_out2, sens1.c_str ());           client.subscribe(topic_out2);
    past_waterpump = current_waterpump;
  } else if (current_waterpump < past_waterpump){
    int status_waterpump = 0;
    String sens1 = String(status_waterpump) + "/" + String(dayStamp) + "/" + String(timeStamp);
    client.publish(topic_out2, sens1.c_str ());           client.subscribe(topic_out2);
    past_waterpump = current_waterpump;
  } else past_waterpump = current_waterpump;

  if (current_waterbackwash > past_waterbackwash){
    int status_waterbackwash = 1;
    String sens1 = String(status_waterbackwash) + "/" + String(dayStamp) + "/" + String(timeStamp);
    client.publish(topic_out3, sens1.c_str ());           client.subscribe(topic_out3);
    past_waterbackwash = current_waterbackwash;
  } else if (current_waterbackwash < past_waterbackwash){
    int status_waterbackwash = 0;
    String sens1 = String(status_waterbackwash) + "/" + String(dayStamp) + "/" + String(timeStamp);
    client.publish(topic_out3, sens1.c_str ());           client.subscribe(topic_out3);
    past_waterbackwash = current_waterbackwash;
  } else past_waterbackwash = current_waterbackwash;

    if (current_feeder > past_feeder){
    int status_feeder = 1;
    String sens1 = String(status_feeder) + "/" + String(dayStamp) + "/" + String(timeStamp);
    client.publish(topic_out4, sens1.c_str ());           client.subscribe(topic_out4);
    past_feeder = current_feeder;
  } else if (current_feeder < past_feeder){
    int status_feeder = 0;
    String sens1 = String(status_feeder) + "/" + String(dayStamp) + "/" + String(timeStamp);
    client.publish(topic_out4, sens1.c_str ());           client.subscribe(topic_out4);
    past_feeder = current_feeder;
  } else past_feeder = current_feeder;

  if (current_aerator > past_aerator){
    int status_aerator = 1;
    String sens1 = String(status_aerator) + "/" + String(dayStamp) + "/" + String(timeStamp);
    client.publish(topic_out5, sens1.c_str ());           client.subscribe(topic_out5);
    past_aerator = current_aerator;
  } else if (current_aerator < past_aerator){
    int status_aerator = 0;
    String sens1 = String(status_aerator) + "/" + String(dayStamp) + "/" + String(timeStamp);
    client.publish(topic_out5, sens1.c_str ());           client.subscribe(topic_out5);
    past_aerator = current_aerator;
  } else past_aerator = current_aerator;

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
