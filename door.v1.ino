/*
 Kuopio Hacklab door code v1
 Currently it stays on wifi ap+ station mode, only station mode needed so should disable currently open AP.
*/

#include <Servo.h>  
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>


const char* ssid     = "ap";
const char* password = "pass";
const int SERVO_LOCK_OPEN = 170;
const int SERVO_LOCK_CLOSE = 20;
Servo doorservo;  // create servo object to control a servo 
unsigned short int debug = 0;
ESP8266WiFiMulti WiFiMulti;

char whitelist[50][10];

void setup() {    
  pinMode(4, OUTPUT);
  pinMode(2, INPUT);

  memset(whitelist, 0, sizeof(whitelist));
  
  beep(200);
  beep(50);
  beep(50);
  
  Serial.begin(9600);

  Serial.println("AT");

  if (debug) 
    Serial.println("Setup()");     
  
  WiFiMulti.addAP(ssid, password);
  //WiFi.begin(ssid, password);

  /*
  while (WiFi.status() != WL_CONNECTED) {
    beep(200);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  */
  beep(50);       
  beep(100);
  
}

void beep(unsigned int delayms){ 
  analogWrite(4, 21);      // Almost any value can be used except 0 and 255
  delay(delayms);          // wait for a delayms ms
  analogWrite(4, 0);      // Almost any value can be used except 0 and 255
  delay(delayms);          // wait for a delayms ms   
}  
void openDoor()
{      
  doorservo.write(SERVO_LOCK_OPEN); 
  beep(750);
  doorservo.write(SERVO_LOCK_CLOSE); 
  beep(1000);
}
// the loop function runs over and over again forever
int atCmd=0;
void loop() {    
  if (Serial.available())
    checkSerialData();
    
  if (WiFiMulti.run() != WL_CONNECTED)
    beep(10);   
  else if (WiFiMulti.run() == WL_CONNECTED && !doorservo.attached())
  {
    if (whitelist[0][0]==0)    
      fetchWhitelist();   
    
    doorservo.attach(10);
    doorservo.write(SERVO_LOCK_CLOSE);      
    
  }

  delay(10);
  
  atCmd++;
  if (atCmd==6000) //6000
  {    
    atCmd=0;
    Serial.println("AT");    
  }    
}
void checkSerialData()
{
  int pos=0;
  char inData[255];
  char inChar;
  
  while(Serial.available() > 0) // Don't read unless                                                
  {
          
          inChar = Serial.read(); // Read a character
          inData[pos] = inChar; // Store it
          pos++; // Increment where to write next          
          if (inChar=='\n')
            break;
            
          beep(10);
  } 
  inData[pos] = '\0'; // Null terminate the string
  String str(inData);
  //+CLIP: "123121234567",145,"",0,"",0
  if (str.substring(0,7).indexOf("+CLIP: ")!=-1)
  {
    String number = str.substring(8,20);
    //somebody is calling
    if (debug)
    {
      Serial.println("Call detected from:");
      Serial.println(number);
    }
    
    //whitelist check        
    if (isWhitelisted(number))
    {
      if (debug)
        Serial.println("Debug: Whitelisted!");
        
      //require physical presense (doorbell pressed)  
      if (digitalRead(2) == 0)    
      {
        Serial.println("ATH");
        openDoor();
        postDoorOpened(number);
      }
      else if(debug)
        Serial.println("Whitelisted but doorbell not pressed");             
    }
    else if (debug)
    {
        Serial.print("Debug: NOT whitelisted: ");
        Serial.println(number);
    }                   
  }
  else if (debug)
  {
    Serial.println("Received data: ");
    Serial.println(str);
  }  
}

//whitelist stuff
bool isWhitelisted(String number)
{
  for (int i=0;i<50;i++)
    if (whitelist[i][0]!=0)         
    {
      String tmp(whitelist[i]);
      if ("358" + tmp == number)
        return true;
    }    
     
    return false;
}
void fetchWhitelist()
{  
    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {
        HTTPClient http;       
        http.begin("url"); //HTTP               
        int httpCode = http.GET();      
        if(httpCode > 0) {                                    
            if(httpCode == HTTP_CODE_OK) {                
                parseStr(http.getString());
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
}
void displayWhitelist()
{
  for (int i=0;i<50;i++)
    if (whitelist[i][0]!=0)
      Serial.println(whitelist[i]);
}
void parseStr(String str)
{
  int n=0;
  char numbers[50][10];
  memset(numbers, 0, sizeof(numbers));
  int pos=0;
  for (int i=0;i<str.length();i++)
  {
    if (str.substring(i, i+1) == ",")
    {      
      str.substring(n, i).toCharArray(numbers[pos],10);      
      //Serial.println(numbers[pos]);
      pos++;      
      n=i+1;            
    }       
  }  
  if (str.length()>10 && str.substring(str.length()-16, str.length()-3) == "1234567890123")
      storeNewWhitelist(numbers);
  else
    Serial.println("Invalid whitelist file!");
 
}
void postDoorOpened(String number)
{  
  if((WiFiMulti.run() == WL_CONNECTED) && whitelist[0][0]!=0) {
        HTTPClient http;            
        http.begin("url"+number); //HTTP               
        int httpCode = http.GET();      
        if(httpCode > 0) {                                    
            if(httpCode == HTTP_CODE_OK) {                
                
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
}
void storeNewWhitelist(char numbers[50][10])
{
  //Serial.println("Storing new whitelist");
  for(int i=0;i<50;i++)
  {
    /*
    if (numbers[i][0]!=0)
    {
      Serial.print("Storing number: "); 
      Serial.println(numbers[i]);    
    }
    else
      Serial.println("Copying empty array"); 
    */
    strncpy(whitelist[i], numbers[i], 10);
    
    //Serial.println(whitelist[i]); 
  }
}


