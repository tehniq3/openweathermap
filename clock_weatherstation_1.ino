/*
 * original from //https://github.com/G6EJD/ESP8266-MAX7219-LED-4x8x8-Matrix-Clock
 * 
 * small changes by Nicu FLORICA (niq_ro)
 * http://www.arduinotehniq.com
 * https://nicuflorica.blogspot.com
 * http://arduinotehniq.blogspot.com
 * ver.2.- add animation on "seconds"
 * ver.2.1 - add reconnect to wi-fi server
 * ver.2.2 - add local language (romanian)
 * ver.3 - add weather info using info from https://roboindia.com/tutorials/nodemcu-weather-station-arduino/
 * 
*/

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>  // https://github.com/markruys/arduino-Max72xxPanel
#include <time.h>
#include <ArduinoJson.h>

int pinCS = 12; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
const byte buffer_size = 45;
char time_value[buffer_size];

// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> GPIO11/MOSI/D7  (Same Pin for WEMOS)
// CS             -> GPIO12/MISO/(D12)  (Same Pin for WEMOS)
// CLK            -> GPIO13/SCK/D5  (Same Pin for WEMOS)

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 70; // In milliseconds

int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels
const char *ssid      = "bbk2";
const char *pass  = "internet2";
byte w = 0; 

char *zi[7] = { "Luni", "Marti", "Miercuri", "Joi", "Vineri", "Sambata", "Duminica" };
char *luna[12] = { "Ianuarie", "Februarie", "Martie", "Aprilie", "Mai", "Iunie", "Iulie", "August", "Septembrie", "Octombrie", "Noiembrie", "Decembrie" };
String APIKEY = "ca55295c4a1dce2688e0751d4b9a68de";       //  Api key from https://openweathermap.org/                              
String CityID = "680332";                                 //Craiova, must put Your City ID

WiFiClient client;
char servername[]="api.openweathermap.org";              // remote server we will connect to
String result;

int  counter = 60;
                                      
String weatherDescription ="";
String weatherLocation = "";
String Country;
float Temperature;
float Humidity;
float Pressure;

void connect_to_WiFi() {  // We start by connecting to a WiFi network
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
/*
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
*/   
for (byte  i = 0; i <= 50; i++) 
   {
   if (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }  
    else
    i = 50;
}
    if (WiFi.status() == WL_CONNECTED) 
    {
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(String(WiFi.localIP()));
    }
}

void setup() {
  Serial.begin(115200);
//  WiFi.begin(ssid,password);
connect_to_WiFi();  // We start by connecting to a WiFi network
  
  configTime(0 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
  setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4",1);
  
  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 1);    // The first display is position upside down
  matrix.setRotation(1, 1);    // The first display is position upside down
  matrix.setRotation(2, 1);    // The first display is position upside down
  matrix.setRotation(3, 1);    // The first display is position upside down
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) 
    {
     connect_to_WiFi();   // We start by connecting to a WiFi network  
    }
  matrix.fillScreen(LOW);
  String time = get_time();
  time.trim();
  Serial.println(time);
  time.substring(0,5).toCharArray(time_value, 10); 
  Serial.println("HH:MM");
  Serial.println(time_value);
  //( Sun  21-07-19 ) ( PM 12:52:12 )
  matrix.drawChar(2,0, time_value[0], HIGH,LOW,1); // H
  matrix.drawChar(8,0, time_value[1], HIGH,LOW,1); // HH
   if (millis()/1000 % 2 == 0)  // animated second
     matrix.drawChar(14,0,time_value[2], HIGH,LOW,1); // HH:
   else
   //  matrix.drawChar(14,0,' ', HIGH,LOW,1); // HH: 
     matrix.drawChar(14,0,'-', HIGH,LOW,1); // HH: 
  matrix.drawChar(20,0,time_value[3], HIGH,LOW,1); // HH:M
  matrix.drawChar(26,0,time_value[4], HIGH,LOW,1); // HH:MM
  matrix.write(); // Send bitmap to display
  delay(500);
  w = w + 1;
  
   if (w == 30)
   {
    String data2 = getWeatherData();
    display_message(data2);
   }
   if (w == 60)
   {
   String data = romana();
   display_message(data);
   w = 0;
   }
} //  end main loop

void display_message(String message){
   for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    //matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background off, reverse to invert the image
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait/2);
  }
}

String get_time(){
  time_t now;
  time(&now);
  char time_output[buffer_size];
  // See http://www.cplusplus.com/reference/ctime/strftime/ for strftime functions
  // Desired format: 10:03:20 
  strftime(time_output, buffer_size, "%T", localtime(&now));   
  return String(time_output); // returns 12:31:45
}

String get_data(){
  time_t now;
  time(&now);
  char time_output1[buffer_size];
  // See http://www.cplusplus.com/reference/ctime/strftime/ for strftime functions
  //  strftime(time_output1, buffer_size, "( %A %d-%m-%Y )", localtime(&now)); // english, short dscription
   strftime(time_output1, buffer_size, "( %A %d-%B-%Y )", localtime(&now)); // english, long description
  //  strftime(time_output1, buffer_size, "( %u %d-%m-%Y )", localtime(&now)); // just number
  // return String(time_output1); // returns ( Sat 20-Apr-19)  
   return String(time_output1); // returns ( Saturday 20-April-2019) 
  // return String(time_output1); // returns ( 6 20-04-2019)
}

String romana(){  // original part writted by nicu FLORICA (niq_ro)
  time_t now;
  time(&now);
  char time_output2[buffer_size];
   strftime(time_output2, buffer_size, "%u", localtime(&now));  // number of Day in Week (1..7)
   int ziua2 = String(time_output2).toInt();
 /*  
   Serial.print("ziua nr. ");
   Serial.print(time_output2);
   Serial.print("- ");
   Serial.print(String(time_output2)); 
   Serial.print("- ");
   Serial.print(ziua2);
   Serial.print("- ");
   Serial.println(zi[ziua2-1]); 
 */  
//  String dataro = (" ( ");
  String dataro = ("  ");
//  if (ziua2 == 7) dataro = dataro + "Duminica";
  dataro = dataro + zi[ziua2-1];
  dataro = dataro + (", ");

  strftime(time_output2, buffer_size, "%d.", localtime(&now)); // day of Month
  dataro = dataro + String(time_output2);
//  dataro = dataro + (" ");

   char time_output3[buffer_size];
   strftime(time_output3, buffer_size, "%m", localtime(&now));
   int luna2 = String(time_output3).toInt();
//   if (luna2 == 9) dataro = dataro + "Septembrie";
  dataro = dataro + luna[luna2-1];

  char time_output4[buffer_size];
//  strftime(time_output4, buffer_size, ".%Y )", localtime(&now)); //year
  strftime(time_output4, buffer_size, ".%Y ", localtime(&now)); //year
  dataro = dataro + String(time_output4); 
  
return dataro; // returns
}


String getWeatherData()                                //client function to send/receive GET request data.
{
  if (client.connect(servername, 80))   
          {                                         //starts client connection, checks for connection
          client.println("GET /data/2.5/weather?id="+CityID+"&units=metric&lang=ro&APPID="+APIKEY);
          client.println("Host: api.openweathermap.org");
          client.println("User-Agent: ArduinoWiFi/1.1");
          client.println("Connection: close");
          client.println();
          } 
  else {
         Serial.println("connection failed");        //error message if no client connect
          Serial.println();
       }

  while(client.connected() && !client.available()) 
  delay(1);                                          //waits for data
  while (client.connected() || client.available())    
       {                                             //connected or data available
         char c = client.read();                     //gets byte from ethernet buffer
         result = result+c;
       }

client.stop();                                      //stop client
result.replace('[', ' ');
result.replace(']', ' ');
Serial.println(result);
char jsonArray [result.length()+1];
result.toCharArray(jsonArray,sizeof(jsonArray));
jsonArray[result.length() + 1] = '\0';
StaticJsonBuffer<1024> json_buf;
JsonObject &root = json_buf.parseObject(jsonArray);

if (!root.success())
  {
    Serial.println("parseObject() failed");
  }

String location = root["name"];
String country = root["sys"]["country"];
float temperatura = root["main"]["temp"];
int humidity = root["main"]["humidity"];
int pressure = root["main"]["pressure"];
String weather = root["weather"]["main"];
String description = root["weather"]["description"];

weatherDescription = description;
weatherLocation = location;
Country = country;
int temperatura0 = (float)10.*temperatura;
int temperatura1 = temperatura0/10;
int temperatura2 = temperatura0%10;  
int pressure1 = pressure * 0.75006;
String vreme = "  ";
  vreme = vreme + location;
  vreme = vreme + ", ";
  vreme = vreme + temperatura1;
  vreme = vreme + ".";  
  vreme = vreme + temperatura2;
  vreme = vreme + "gr.C ";
  vreme = vreme + humidity;
  vreme = vreme + "%RH ";  
  vreme = vreme + pressure1;
  vreme = vreme + "mmHg, ";
  vreme = vreme + description;
  vreme = vreme + "  ";    
  
  return vreme; // returns
}

void madeWeather(String location,String description)
{
  Serial.println(" ");
  Serial.print(location);
  Serial.print(", ");
  Serial.print(Country);
  Serial.print(" - ");
  Serial.println(description);
}

void  madeConditions(float Temperature,float Humidity, float Pressure)
{
 Serial.println(" ");                        //Printing Temperature
 Serial.print("T:"); 
 Serial.print(Temperature,1);
 Serial.println("^C "); 
                                         
 Serial.print("H:");                       //Printing Humidity
 Serial.print(Humidity,0);
 Serial.println(" %"); 
 
                      //Printing Pressure
 Serial.print("P: ");
 Serial.print(Pressure,1);
 Serial.println(" hPa");
}
