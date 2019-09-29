// Robo India Tutorials - https://roboindia.com/tutorials/nodemcu-weather-station-arduino/
// Hardware: NodeMCU
// simple Code for reading information from openweathermap.org 
// code cleaned by Nicu FLORICA (niq_ro)

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

const char* ssid     = "WiFi name";                 // SSID of local network
const char* password = "password";                    // Password on network
String APIKEY = "ca55295c4a1dblablabla4b9a68de";                                 
String CityID = "680332";                                 //Your City ID, see https://openweathermap.org/find?q=

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


void setup() {
  Serial.begin(115200);
//  int cursorPosition=0;

  Serial.println("Connecting");
  WiFi.begin(ssid, password); 
             while (WiFi.status() != WL_CONNECTED) 
            {
            delay(500);
            Serial.print(".");
            }
  Serial.println("Connected");
  delay(1000);
}

void loop() {
    if(counter == 60)                                 //Get new data every 10 minutes
    {
      counter = 0;
      displayGettingData();
      delay(1000);
      getWeatherData();
    }else
    {
      counter++;
      displayWeather(weatherLocation,weatherDescription);
      delay(5000);
      displayConditions(Temperature,Humidity,Pressure);
      delay(5000);
    }
}

void getWeatherData()                                //client function to send/receive GET request data.
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
float temperature = root["main"]["temp"];
float humidity = root["main"]["humidity"];
String weather = root["weather"]["main"];
String description = root["weather"]["description"];
float pressure = root["main"]["pressure"];
weatherDescription = description;
weatherLocation = location;
Country = country;
Temperature = temperature;
Humidity = humidity;
Pressure = pressure;

}

void displayWeather(String location,String description)
{
  Serial.println(" ");
  Serial.print(location);
  Serial.print(", ");
  Serial.print(Country);
  Serial.print(" - ");
  Serial.println(description);
}

void displayConditions(float Temperature,float Humidity, float Pressure)
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
 Serial.print(" hPa");
}

void displayGettingData()
{
  Serial.println("Getting data...........");
}


