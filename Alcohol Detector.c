#include <SoftwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>

#define BUTTON_PIN 4

#define gas A0 
#define buzzer 8
#define relay 2

#define in1 10// L298N Motor Driver pins.
#define in2 11
#define in3 12
#define in4 13
#define ena 9
#define enb 3
IRrecv IR(0);
 
decode_results rslts;
SoftwareSerial mySerial(5, 4); //SIM800L Tx & Rx is connected to Arduino #5 & #4

LiquidCrystal_I2C lcd(0x27, 16, 2);

SoftwareSerial GPSModule(7, 6); // RX,TX)

String readGPSData();

int alc=0;
int Speed = 100; // Motor speed (0 - 255)

void setup() {
  
  pinMode(gas,INPUT);
  pinMode(relay,OUTPUT);
  pinMode(buzzer,OUTPUT); 
  digitalWrite(relay,LOW);
  digitalWrite(buzzer,LOW);
  
  lcd.init(); // Initialize the LCD
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(ena,OUTPUT);
  pinMode(enb,OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  IR.enableIRIn();
  // Initialize serial communication with Bluetooth module
  Serial.begin(9600); 
  GPSModule.begin(9600);    // Initialize serial communication for debugging (optional)
  mySerial.begin(9600);
  

}
void sendsms(String phoneNumber, String message) {
  Serial.println("Initializing SMS...");
  delay(1000);

  mySerial.println("AT"); // Handshake
  updateSerial();

  mySerial.println("AT+CMGF=1"); // Set SMS to text mode
  updateSerial();

  mySerial.print("AT+CMGS=\"");
  mySerial.print(phoneNumber); // Phone number
  mySerial.println("\"");
  updateSerial();
  delay(1000); // Delay for modem response

  mySerial.print(message); // Message content
  updateSerial();

  mySerial.write(26); // Ctrl-Z, end of message
  delay(1000); // Delay for modem response
  digitalWrite(buzzer, LOW);
  timer();
}


void loop() {
  
  

   if (detectalcohal()){
    digitalWrite(relay,LOW);
    drive();
    }
    
  
      
    
}

void drive(){
    
    if (IR.decode()){
        Serial.print("drivee");
        Serial.println(IR.decodedIRData.decodedRawData, HEX);
      
        switch (IR.decodedIRData.decodedRawData) {
        
         case  0xE718FF00:            //  FORDWARD Button
            Forward();
            break;
 
         case 0xA55AFF00:            //  LEFT Button
            Left();
            break;   
 
         case 0xF708FF00:            //  RIGHT Button
            Right();
            break;
 
         case 0xAD52FF00:            //  BACK Button
            Back();
            break;
        
         case 0xE31CFF00:            // STOP Button
            Stop();
            break;
   }
        IR.resume();
    }
}


int seatbelt(){
  int seatbeltState = digitalRead(BUTTON_PIN);
Serial.println(seatbeltState);
  if (seatbeltState == 0) {
    // Turn off other LEDs
  
    
    return 1;
  }
  else {
    // Turn off the inbuilt LED
    digitalWrite(relay,LOW);
    return 0;
  }
}

int detectalcohal(){
    alc=digitalRead(gas);
  if(alc==HIGH)
  { 
    digitalWrite(buzzer,LOW);
    
    digitalWrite(relay,LOW);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" Safe ");
    lcd.setCursor(0,1);
    lcd.print("Drive... ");
      
    
    digitalWrite(buzzer,LOW);  
    return 1;                   
  }
  
  else {
    digitalWrite(relay, HIGH);
    digitalWrite(buzzer, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("ALCOHOL");
    lcd.setCursor(0, 1);
    lcd.print("DETECTED");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Sending");
    lcd.setCursor(0, 1);
    lcd.print("Sms");

    // Send SMS
    sendsms("+919573162285", "Alcohol detected of your mate at:" + gpsCoords()); // Replace with your desired phone number and message
    return 0;
  
  }
}



String readGPSData() {
    GPSModule.flush(); // Clear any data received from the GPS module.
    while (GPSModule.available() > 0) {
        GPSModule.read(); // Read data from the GPS module.
    }

    String data = "";
    if (GPSModule.find("$GPRMC,")) {
        String tempMsg = GPSModule.readStringUntil('\n');
        data = tempMsg;
    }
    return data;
}

String gpsCoords() {
    String latitude = "";
    String longitude = "";

    String gpsData = readGPSData(); // Function to read GPS data; ensure this is defined elsewhere.

    if (gpsData != "") {
        // Assuming latitude is the third element and longitude is the fifth element in the GPS data.
        int firstCommaIndex = -1;
        for (int i = 0; i < 3; i++) { // Find the comma before the latitude
            firstCommaIndex = gpsData.indexOf(',', firstCommaIndex + 1);
        }
        int secondCommaIndex = gpsData.indexOf(',', firstCommaIndex + 1);
        latitude = gpsData.substring(firstCommaIndex + 1, secondCommaIndex);

        firstCommaIndex = secondCommaIndex;
        secondCommaIndex = gpsData.indexOf(',', firstCommaIndex + 1);
        longitude = gpsData.substring(firstCommaIndex + 1, secondCommaIndex);

        Serial.print("Latitude: ");
        Serial.println(latitude);
        Serial.print("Longitude: ");
        Serial.println(longitude);
    } else {
        Serial.println("No GPS data available.");
    }

    // Create the URL for Google Maps.
    if (latitude != "" && longitude != "") {
        return "https://www.google.com/maps?q=" + latitude + "," + longitude;
    } else {
        return "Invalid coordinates";
    }

    delay(1000); // Note: Having a delay here may not be necessary or desired, depending on usage context.
}



void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

void timer(){
  for(int i = 10;i>-1;i--){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Engine Locked");
    lcd.setCursor(0, 1);
    lcd.print(i);
    delay(1000);
  }

}
void Back()
{
            digitalWrite(enb, HIGH);
            digitalWrite(ena, HIGH);
            digitalWrite(in3, HIGH);
            digitalWrite(in4, LOW);
            digitalWrite(in1, HIGH);
            digitalWrite(in2, LOW);
}
void Forward()
{ 
            digitalWrite(enb, HIGH);
            digitalWrite(ena, HIGH);
            digitalWrite(in2, HIGH);
            digitalWrite(in4, HIGH);
            digitalWrite(in1, LOW);
            digitalWrite(in3, LOW);
            
}    
void Left()
{
            digitalWrite(enb, HIGH);
            digitalWrite(in4, HIGH);
            digitalWrite(in2, LOW);           
            
}
 
void Right()
{
            
            digitalWrite(ena, HIGH);
            digitalWrite(in2, HIGH);
            digitalWrite(in4, LOW);
            
} 
void Stop()
{
            digitalWrite(enb, LOW);
            digitalWrite(ena, LOW);
            digitalWrite(in1, LOW);
            digitalWrite(in2, LOW);
            digitalWrite(in3, LOW);
            digitalWrite(in4, LOW);
}
