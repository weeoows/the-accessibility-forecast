// The Accessibility Forecast - Read sensor data from thingspeak

#include <LiquidCrystal.h>
#include "WiFiEsp.h"
#include "secrets_read.h"
#include "ThingSpeak.h"

// WiFi information
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiEspClient  client;

// LCD Pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Emulate Serial1 on pins 6/7 if not present. (ESP-01 module)
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#define ESP_BAUDRATE  19200
#else
#define ESP_BAUDRATE  115200
#endif

// Counting channel details
unsigned long counterChannelNumber = SECRET_CH_ID;
const char * myCounterReadAPIKey = SECRET_READ_APIKEY;
unsigned int temp_field = 1; 
unsigned int humidity_field = 2; 
unsigned int volume_field = 3; 
unsigned int light_field = 4; 
// Status of parameters
String temp_Status = "";
String hum_Status = "";
String vol_Status = "";
String lig_Status = "";

void setup() {

  lcd.begin(16, 2); // initialising LCD
  
 //Initialize serial and wait for port to open
  Serial.begin(115200);  
  while(!Serial){
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  
  // initialize serial for ESP module  
  setEspBaudRate(ESP_BAUDRATE);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }

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
    
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

// Main Program Loop
void loop() 
{
  int statusCode = 0;
  
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected");
  }

  // Read each field in channel into variables
  float temp   = ThingSpeak.readFloatField(counterChannelNumber, temp_field, myCounterReadAPIKey);
  float humidity = ThingSpeak.readFloatField(counterChannelNumber, humidity_field, myCounterReadAPIKey); 
  int volume = ThingSpeak.readFloatField(counterChannelNumber, volume_field, myCounterReadAPIKey);
  int light = ThingSpeak.readFloatField(counterChannelNumber, light_field, myCounterReadAPIKey); 

  // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Temperature: " + String(temp) + " deg C");
    Serial.println("Humidity: " + String(humidity) + " %");
    Serial.println("Volume: " + String(volume) + " db");
    Serial.println("Brightness: " + String(light) + " Lux");
  }
  else{
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }

  if(temp>27.00)
  {
    temp_Status = "Warm";
  }
  if(temp>=21.00 && temp <= 25.50)
  {
    temp_Status = "Optimal";
  }
  if(temp < 21.00)
  {
    temp_Status = "Cold";
  }

  if(humidity<29.00)
  {
    hum_Status = "Low";
  }
  if(humidity>=29.00 && humidity<=55.50)
  {
    hum_Status = "Optimal";
  }
  if(humidity > 55.5)
  {
    hum_Status = "High";  
  }

  if(volume<30)
  {
    vol_Status = "Quiet";
  }
  if(volume>=30 && volume <= 45)
  {
    vol_Status = "Optimal";
  }
  if(volume > 45)
  {
    vol_Status = "Noisy";
  }

  if(light<380)
  {
    lig_Status = "Dim";
  }
  if(light>=380 && light<=500)
  {
    lig_Status = "Optimal";
   }
  if(light > 500)
  {
    lig_Status = "Bright";
  }

  //Display parmeter status on LCD
  lcd.clear(); // clear LCD before displaying
  lcd.setCursor(0,0); 
  lcd.print(temp_Status);
  lcd.setCursor(0,1);
  lcd.print(hum_Status);
  lcd.setCursor(9,0);
  lcd.print(vol_Status);
  lcd.setCursor(9,1);
  lcd.print(lig_Status);
  
  
  delay(50000); // No need to read the read variables too often.
}

// This function attempts to set the ESP8266 baudrate. Boards with additional hardware serial ports
// can use 115200, otherwise software serial is limited to 19200.
void setEspBaudRate(unsigned long baudrate){
  long rates[6] = {115200,74880,57600,38400,19200,9600};

  Serial.print("Setting ESP8266 baudrate to ");
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
