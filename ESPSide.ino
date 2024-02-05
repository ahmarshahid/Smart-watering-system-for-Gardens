#include <WiFi.h>
#include <PubSubClient.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


// # define the firebase realtime database API Key and DATABASE URL
#define API_KEY "AIzaSyBshho8Db3vcAsDUglSZyBkBssOy68RuMA"
#define DATABASE_URL "https://coal-lab-default-rtdb.europe-west1.firebasedatabase.app/"

//#define mqttServer "broker.hivemq.com"
const char *ssid = "Homé";
const char *password = "90009000";
const char *mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char *mqttClientId = "COAL_PROJECT_ID_16.222";
const char *outputTopic = "esp32/output_16.222";
const char *inputTopic = "esp32/input_16.222";
static unsigned long lastMillis = 0;


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

// #define the ThingSpeak Module
const char *thingSpeakApiKey = "VPRIHK24BN2YUILV";
const char *thingSpeakUrl = "https://api.thingspeak.com/update?api_key=VPRIHK24BN2YUILV&field1=0";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  WifiSetup();

  // Configure MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callBack);
  connectToMQTT();

   /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void WifiSetup(){
  // Connect to Wi-Fi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("WiFi connected with IP address: ");
  Serial.println(WiFi.localIP());
}

void callBack(char* inputTopic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(inputTopic);
  Serial.print(". Message : ");
  String messageTemp;
  char tempPressed;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.println(messageTemp);
  Serial.print("Message Sent to UART : ");
}


void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WifiSetup();
  }
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis >
4000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

// Now use buttonChar in your Firebase.RTDB.setInt call
    if (Firebase.RTDB.setInt(&fbdo, "data/sensor", 12))
    {
      Serial.print("PASSED   to ");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }


  if(Serial2.available()> 0)
  {
     //String receivedChar = Serial2.readString();
     //Serial.print("Message Received through UART  :   ");
     //Serial.println(receivedChar);
  }

  // Handle MQTT events
  if (!client.connected()) {
    connectToMQTT();
  }
  client.loop();

  // Message Publishing on app with delay of 5s
  if (millis() - lastMillis > 5000) {


    const char* message = "Local Server Activated";
    publishMessage(message);
    lastMillis = millis();
  }

  // Data Sending to FireBase
}


void connectToMQTT() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(mqttClientId)) {
      Serial.println("Connected to MQTT");
      client.subscribe(inputTopic);
    }
    else {
      Serial.print("Failed with state   :  ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void publishMessage(const char* message) {
  if (client.connected()) {
    client.publish(outputTopic, message);
    Serial.print("Message Published :  ");
    Serial.println(message);
  }
}
