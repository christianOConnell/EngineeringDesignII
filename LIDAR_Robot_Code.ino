
// Public library preinstalled in Arduino IDE
#include <Ultrasonic.h>
#include <Servo.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

// Private library, need to have them in the main program folder
#include "PubSubClient.h" 
#include "info.h"
#include "WiFiManager.h"
#include "SSD1306.h"

//MQTT Communication associated variables
String inString = "";
int testvariable;
char payload_global[100];
boolean flag_payload;
int k = 0; // done target constant

//MQTT Setting variables
//Info dashed out for security
const char* mqtt_server = "-----";
const int mqtt_port = ---;
const char* MQusername = "----";
const char* MQpassword = "----";

//WiFi Define
WiFiClient espClient;
info board_info;
PubSubClient client(espClient);

////////////////////////////////////////Robot Logic Variables//////////////////////////////////////////////////
//////////////////////////Students change this section for their modification//////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////// Wifi Settings variables // Remote students change to your network parameters ///////////////////////
const char* ssid = "---";   //Dashed out for security
const char* password = "---";

////////// If on-campus, remember to add your WeMos MAC address to Stevens network ////////////////////////////

//////********CHANGE FOR EACH ARENA***********////////
const char* MQtopic = "louis_lidar2"; // Topic for Arena_2 
//const char* MQtopic = "louis_lidar3"; // Topic for Arena_3 

// Define the DC motor contorl signal pins
#define motorRpin D0  //GPIO pin setting for motorR
#define motorLpin D2  //GPIO pin setting for motorL

Servo motorR;        //Create servo motorR object to control a servo
Servo motorL;        //Create servo motorL object to control a servo

// Define the Ultrasonic sensor pins
Ultrasonic ultrasonic_front(D8, D5); // Ultasonic sensor, front (trig, echo)
Ultrasonic ultrasonic_right(D9, D6); // Ultasonic sensor, right (trig, echo)
Ultrasonic ultrasonic_left(D10, D7); // Ultasonic sensor, left (trig, echo)

// Define the OLED display pins D14 SDA, D15 SCL
SSD1306  display(0x3C, D14, D15); //Address set here 0x3C
                                  //Pins defined as D2 (SDA/Serial Data) and D5 (SCK/Serial Clock).

////////////////////////// Define the variables needed for your algorithm ////////////////////////////////////

int distance_left = 0;
int distance_right = 0;
int distance_front = 0;
int len_dist_array = 0;
int shortest_dist = 0;
int side_to_avoid = 0;
long spin_side = 1;
int side_bubble = 50; // robot bubble area in mm
int front_bubble = 70; // robot bubble area in mm
int current_target = 0;
int target_bubble = 100;
float dist_to_target = 2000;
int motorR_PWM = 90;
int motorL_PWM = 90;
double previous_x = -1; // The previous coordinate
double previous_y = -1; // The previous coordinate
double prev_target_x = -1;
double prev_target_y = -1;
double a = -1;
double b = -1;
int c = -1;
double dir = 0;
int xDiff = 0;
int yDiff = 0;
boolean xSolve = true;
float x = 0;
float y = 0;

////////////////////////////////////Robot Logic Variables End//////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Setup the wifi, Don't touch
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  randomSeed(micros());
}

// MQTT msg callback
void callback(char* topic, byte* payload, unsigned int length) {
  for (int i = 0; i < length; i++) {
    payload_global[i] = (char)payload[i];
  }
  payload_global[length] = '\0';
  flag_payload = true;
}

// MQTT reconnection setup
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQusername,MQpassword)) {
       client.subscribe(MQtopic);
    }
  }
}

/////////////////////////////// SETUP LOOP. Don't Touch ///////////////////////////////////////////

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  motorR.attach(motorRpin);   //motorR is attached using the motorRpin
  motorL.attach(motorLpin);   //motorL is attached using the motorLpin
  // Display Setup
  display.init();
  display.flipScreenVertically();
  display.drawString(0, 0, "Group 5 Robot"); //Just dummy code for debugging; feel free to change this
  display.display();
}

/////////////////////////////////// MAIN LOOP /////////////////////////////////////////////

void loop() {
  //subscribe the data from mqtt server
  if (!client.connected()) {
      reconnect();
  }
  const char *msg = "target";
  char temp1[50];
  sprintf(temp1,"%d",k);
  const char *c = temp1;
 
  client.publish( msg , c);
  client.loop();
  String payload(payload_global);
 
  int testCollector[10];
  int count = 0;
  int prevIndex, delimIndex;
 
  prevIndex = payload.indexOf('[');
  while( (delimIndex = payload.indexOf(',', prevIndex +1) ) != -1){
    testCollector[count++] = payload.substring(prevIndex+1, delimIndex).toInt();
    prevIndex = delimIndex;
  }
  delimIndex = payload.indexOf(']');
  testCollector[count++] = payload.substring(prevIndex+1, delimIndex).toInt();
 
  // Robot location x,y from MQTT subscription variable testCollector
 // float x, y;
  x = testCollector[0];
  y = testCollector[1];

  //Setting up the target destination, xt[]={A,B,C,D ~}
  int xt[] = {1000,500,100,1700};
  int yt[] = {200,100,100,300};

////////////////////////////////////////Robot Logic Begin//////////////////////////////////////////////////
////////////////////////Students change this section for their modification////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////// find the closest obstacle ///////////////////////////////////////

  // Read the distances from the Utrasonic sensors. Output unit is mm
  distance_left = ultrasonic_left.read(CM)* 10;
  distance_front = ultrasonic_front.read(CM)* 10;
  distance_right = ultrasonic_right.read(CM)* 10;

  //////////////////// New coordinates event ///////////////////////////////////////////

  


    
    // Display the x,y location in the OLED
    display.clear(); // Clear the buffer
    String str_1 = "x: " + String(x); // We need to cast the x value from 'int' to 'String'
    String str_2 = "y: " + String(y); // Same thing for the y value
    display.drawString(0, 0, str_1);
    display.drawString(0, 15, str_2);
    String str_3 = "Rdis: " + String(distance_right); 
    display.drawString(0, 30, str_3);
    String str_4 = "Ldis: " + String(distance_left);
    display.drawString(0, 45, str_4);
    display.display();
  

  //////////////////// Object avoidance algorithm /////////////////////////////////////

 if (distance_front < front_bubble) {
  motorR.write(90); //stop to give the sensors a better chance to read good values
  motorL.write(90);
  delay(450);
  distance_left = ultrasonic_left.read(CM)* 10;
  distance_front = ultrasonic_front.read(CM)* 10;
  distance_right = ultrasonic_right.read(CM)* 10;
  delay(50);
    if (distance_left > distance_right) {
      spin_side = 1; // LFT = 1, RGHT = 2
    }
    else {
      spin_side=2;
    }
 
    if (spin_side == 1) { // Rotate to the left - counterclockwise
      turn("left");
      delay(230);
      getDists();
      while(distance_right < 2*front_bubble) {
         motorR.write(85);  // moving until front of the robot is past the obstacle
         motorL.write(98);
         distance_right = ultrasonic_right.read(CM)* 10;
         displayVals();
         delay(50);
      }
      motorR.write(85); //go a bit farther to clear the object
      motorL.write(98);
      delay(1900);
      turn("right");
      delay(240);
      distance_right = ultrasonic_right.read(CM)* 10;
      while(distance_right < 2*front_bubble && ~(abs(x-xt[current_target]) <= 11 && xSolve == true) || distance_right < 2*front_bubble && ~(abs(y-yt[current_target]) <= 11 && xSolve == false))
     // while(distance_right < 2*front_bubble && !(abs(x-xt[current_target]) <= 11 && xSolve == true) || distance_right < 2*front_bubble && !(abs(y-yt[current_target]) <= 11 && xSolve == false)) 
      { //this while loop checks to see if you've aligned in an axis with the target during it; if you used the 1st one you would be perpendicular to the way we want to go
         motorR.write(85);  // moving until front of the robot is past the obstacle
         motorL.write(98);
         distance_right = ultrasonic_right.read(CM)* 10; //to check if robot is still next to an obstacle
         getCoords(); //to check if robot is near the target
         displayVals();
         delay(100);
      }
      if(~(abs(x-xt[current_target]) <= 11 && xSolve == true) || ~(abs(y-yt[current_target]) <= 11 && xSolve == false))
      { //if you are near the target you don't need to keep moving and turning so we skip this part if we are near the target (on an axis)
      motorR.write(85); //go a bit farther to clear the object
      motorL.write(98);
      delay(1900);
      turn("right");
      delay(240);
      motorR.write(85); //go a bit farther to clear the object
      motorL.write(98);
      delay(1900);
      turn("left");
      delay(230);
      motorR.write(85); //reset motor values
      motorL.write(98);
      }
    }
    else { // Rotate to the right - clockwise
      turn("right");
      delay(240);
      getDists();
      while(distance_left < 2*front_bubble) { // might have to change front_bubble to a different value here
         motorR.write(85);  // moving until front of the robot is past the obstacle
         motorL.write(98);
         distance_left = ultrasonic_left.read(CM)* 10;
         displayVals();
         delay(100);
      }
      motorR.write(85); //go a bit farther to clear the object
      motorL.write(98);
      delay(1900);
      turn("left");
      delay(230);
      getDists();
      while(distance_left < 2*front_bubble && ~(abs(x-xt[current_target]) <= 11 && xSolve == true) || distance_right < 2*front_bubble && ~(abs(y-yt[current_target]) <= 11 && xSolve == false)) { // might have to change front_bubble to a different value here
         motorR.write(85);  // moving until front of the robot is past the obstacle
         motorL.write(98);
         distance_left = ultrasonic_left.read(CM)* 10;
         getCoords();
         displayVals();
         delay(100);
      }
      if(~(abs(x-xt[current_target]) <= 11 && xSolve == true) || ~(abs(y-yt[current_target]) <= 11 && xSolve == false))
      {
      motorR.write(85); //go a bit farther to clear the object
      motorL.write(98);
      delay(1900);
      turn("left");
      delay(230);
      motorR.write(85); //go a bit farther to clear the object
      motorL.write(98);
      delay(1900);
      turn("right");
      delay(240);
      motorR.write(85); //reset motor values
      motorL.write(98);
      }
    }
  }
  else if (distance_right < side_bubble) {   
    motorR.write(80); // move away from object
    motorL.write(98);
  }
  else if (distance_left < side_bubble) {  
    motorR.write(85); // move away from object
    motorL.write(105);
  }
  else if ((abs(x-xt[current_target]) > 11 && xSolve == true) || (abs(y-yt[current_target]) > 11 && xSolve == false)) {  
    motorR.write(85); // move forward
    motorL.write(98);
  }
  else if ( abs(x-xt[current_target]) <= 11 && xSolve == true) {
    xSolve = false;
    if(xDiff > 0){
      if(y-yt[current_target] > 0){
        turn("right");
        delay(240);
        yDiff = -1;
      }
      else {
         turn("left");
         delay(230);
         yDiff = 1;
      }
    }
    else {
      if(y-yt[current_target] < 0){
        turn("right");
        delay(240);
        yDiff = 1;
      }
      else {
         turn("left");
         delay(230);
         yDiff = -1;
      }
    }
  }
   else if ( abs(y-yt[current_target]) <= 11 && xSolve == false) {
    xSolve = true;
    if(yDiff > 0){
      xDiff=1;
      if(x-xt[current_target] < 0){
        turn("right");
        delay(240);
        
      }
      else {
        turn("left");
        delay(230);
        
      }
    }
    else {
      xDiff = -1;
      if(x-xt[current_target] > 0){
        turn("right");
        delay(240);
      }
      else {
         turn("left");
         delay(230);
      }
    }
  }
  //////////////////// Maze solving - Target finding ///////////////////////////////////

  // If we reach the target, lets set the next target
  dist_to_target = sqrt(sq(x-xt[current_target])+sq(y-yt[current_target]));
  if (dist_to_target < target_bubble) {
    motorR.write(90); // Stop the robot for 2.5sec and then continue the search
    motorL.write(90);
    current_target ++;
    prev_target_x = x;
    prev_target_y = y;
    delay(2500);
    if (current_target > 3) { // Repeat the search after 9 sec
      delay(9000);
      current_target = 0;
    }
    motorR.write(101); //back up a bit
    motorL.write(82);
    delay(700);
    motorR.write(90); //reset motors to be safe
    motorL.write(90);
    prevIndex = payload.indexOf('[');
    while( (delimIndex = payload.indexOf(',', prevIndex +1) ) != -1){
     testCollector[count++] = payload.substring(prevIndex+1, delimIndex).toInt();
      prevIndex = delimIndex;
     }
    delimIndex = payload.indexOf(']');
    testCollector[count++] = payload.substring(prevIndex+1, delimIndex).toInt();
 
     // Robot location x,y from MQTT subscription variable testCollector
  //   float x, y;
     x = testCollector[0];
     y = testCollector[1];
     a = x - prev_target_x;          //a b and c for pythag theorem + triangulation                   
              //flip every b value after finishing the code; update: should be done now
     b = y - prev_target_y;
     /* +a -b = angle is east of north
      -a -b = angle is west of north
      +a +b = angle is east of south
      -a +b = angle is west of south */ //the b values might be flipped in this comment because we assumed the wrong corner was (0,0), but if so we flipped the b values in the if statements to undo it
    if(xt[current_target]>x){
      xDiff = 1;
    }
    else {
      xDiff = -1;
    }
    if (a<0 && b> 0){
    
      dir =atan(a/b);
      if(xDiff>0){
        
       turnX((90-dir)*-1);// turn 90-dir left
      }
       else {
        turn("right");
        turnX(dir);//Also turn dir right afterwards  
       }
       
        
    }
    else if(a<0 && b<0){
      dir =atan(a/b);  
      if(xDiff>0){
        turn("left");
        turnX(dir*-1);//Also turn dir left afterwards
      }
       else {
        turnX(90-dir);//turn 90-dir right
       }
    }
    
    else if(a>0 && b> 0){
      dir = atan(a/b);
      turnX(dir*-1);//turn dir left
      if(xDiff>0){
        turn("left");
      }
       else {
        turn("right");
       }
    }
    
    else if (a>0 && b<0){
      dir = atan(a/b);
      turnX(90-dir);//90-dir right
      if(xDiff>0){
        turn("right");
      }
       else {
        turn("left");
       }
    }
  }
}
    
  
    
  
    
  void turn(String way){
      motorL.write(90);
      motorR.write(90);
      delay(250);
   if(way.equals("left")==true){
      motorL.write(66); //turn
      motorR.write(66);
      delay(210);
      motorL.write(90);
      motorR.write(90);
      delay(350);
  }
   else if(way.equals("right")==true){
      motorL.write(112); //turn
      motorR.write(112);
      delay(240);
      motorL.write(90);
      motorR.write(90);
      delay(350);
   }

  }
void turnX(int degX) {  // negative -> turn left; positive -> turn right
  if (degX < 0) {
  motorL.write(68); 
  motorR.write(68);
  if (degX > -10) {delay(4.2*10); }
  else if (degX > -21) { delay(4.2*abs(degX)); }
  else if (degX > -38) { delay(3*abs(degX)); }
  else {delay(2.6*abs(degX));}
  motorL.write(90); 
  motorR.write(90);
  delay(500);
  }
  else {
  motorL.write(113); 
  motorR.write(113);
  if (degX < 10) { delay(4.2*10); }
  else if (degX <21){ delay(4.2*abs(degX)); }
  else if (degX < 38) { delay(3.2*abs(degX)); }
  else {delay(2.7*abs(degX));}
  motorL.write(90); 
  motorR.write(90);
  delay(500);
  }
}
void getCoords() {
  String payload(payload_global);
 
  int testCollector[10];
  int count = 0;
  int prevIndex, delimIndex;
 
  prevIndex = payload.indexOf('[');
  while( (delimIndex = payload.indexOf(',', prevIndex +1) ) != -1){
    testCollector[count++] = payload.substring(prevIndex+1, delimIndex).toInt();
    prevIndex = delimIndex;
  }
  delimIndex = payload.indexOf(']');
  testCollector[count++] = payload.substring(prevIndex+1, delimIndex).toInt();
 
  // Robot location x,y from MQTT subscription variable testCollector
 // float x, y;
  x = testCollector[0];
  y = testCollector[1];

}
void getDists() {
  distance_left = ultrasonic_left.read(CM)* 10;
  distance_front = ultrasonic_front.read(CM)* 10;
  distance_right = ultrasonic_right.read(CM)* 10;
}
void displayVals() {
    display.clear(); // Clear the buffer
    String str_1 = "x: " + String(x); // We need to cast the x value from 'int' to 'String'
    String str_2 = "y: " + String(y); // Same thing for the y value
    display.drawString(0, 0, str_1);
    display.drawString(0, 15, str_2);
    String str_3 = "Rdis: " + String(distance_right); 
    display.drawString(0, 30, str_3);
    String str_4 = "Ldis: " + String(distance_left);
    display.drawString(0, 45, str_4);
    display.display();
}
