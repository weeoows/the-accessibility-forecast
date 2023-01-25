// The Accessibility Forecast - Send sensor data to thingspeak

#include <WiFiEsp.h>
#include <dht_nonblocking.h>
#include "secrets_write.h"
#include "ThingSpeak.h"

// Wifi Information
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiEspClient  client;
// Thingspeak Information
unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

//Variables and set up for sensors:
String myStatus = "";
// DHT sensor
#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 2;
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
// Sound sensor
int mic_pin = A0;
long Sound_signal; 
int num_Measure = 128;
long volume = 0;
// (Photoresistor) Light sensor
int light = 0; // store the current light value

// Emulate Serial1 on pins 6/7 if not present (ESP-01 module)
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#define ESP_BAUDRATE  19200
#else
#define ESP_BAUDRATE  115200
#endif

//ESP Baud Rate
void setEspBaudRate(unsigned long baudrate){
  long rates[6] = {115200,74880,57600,38400,19200,9600};

  Serial.print("Setting ESP8266 baudrate to ");     // initialising ESP shown in serial monitor
  Serial.print(baudrate);
  Serial.println("...");

  for(int i = 0; i < 6; i++){
    Serial1.begin(rates[i]);
    delay(100);
    Serial1.print("AT+UART_DEF=");
    Serial1.print(baudrate);
    Serial1.print(",8,1,0,0\r\n");
    delay(100);  
  }
    
  Serial1.begin(baudrate);
}

//Serial - Checking for ESP module
void setup( )
{
  pinMode(A0, INPUT);

  Serial.begin( 115200 );
  setEspBaudRate(ESP_BAUDRATE);

  Serial.print("Searching for ESP8266..."); 
  // initialize ESP module
  WiFi.init(&Serial1);

// check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  Serial.println("found it!");
   
  ThingSpeak.begin(client);
}

// Main Program Loop
void loop( )
{
  float temperature;
  float humidity;

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if the network used is open or a WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  // Measure sound volume
  long sum = 0; // sum to be reset every loop
  for ( int i = 0 ; i <num_Measure; i ++)  
  {  
   Sound_signal = analogRead (mic_pin);  
    sum =sum + Sound_signal;  
  }  
  volume = 15 + sum / num_Measure; // Calculate the average value  
  Serial.print (volume,1);  
  Serial.print( " dB" );


  // Measure light Intensity
  light = analogRead(A1);
  Serial.print( light, 1 );
  Serial.println( " Lux" );
  
  // Measure Temperature and Humidity
  dht_sensor.measure( &temperature, &humidity );
  Serial.print( "T = " );
  Serial.print( temperature, 1 );
  Serial.print( " deg. C, H = " );
  Serial.print( humidity, 1 );
  Serial.println( "%" );
  //}

  if(temperature==0.0 || humidity==0.0 || volume==0 || light==0)     // Avoiding uploading of outliers during sensor initialisation 
  {
    Serial.println("0 value not uploaded.");
  }
  else
  {
    ThingSpeak.setField(1, temperature);                            // Otherwise, channels are updated
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setField(3, volume);
    ThingSpeak.setField(4, light);
  }

  
  // Write to Thing Speak
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  delay(20000);
}