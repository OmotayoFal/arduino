#include <Wire.h>
#include <Arduino.h>
#include <TimeLib.h>
#include <TimerOne.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>


#include <MemoryFree.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

#define synchronisation 0
#define main 1
long del_Char = 0;


unsigned long selectPressStartTime = 0;  //start time from when select button is pressed
bool selectButtonPressed = false;
int state = main;
int i = 0;  //for index of vehicle list
static bool change = true;
int index = 0;                   //index of vehicles in list
unsigned long startPressed = 0;  // the moment the button was pressed
int endPressed = 0;
int idleTime;

byte customCharUp[8] = {
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};
byte customCharDown[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b10101,
  0b01110,
  0b00100
};


// CREATE CLASS FOR DIFFERENT TYPES OF VEHICLES
class Vehicle {
public:  // Add public access specifier
  char type;
  String registrationNumber;
  String parkingLocation;
  String entryTimeStamp;
  String exitTimeStamp;
  String paymentIndicator;
  //Default constructor
  /*Vehicle() {
    // Initialize default values or leave them empty
    type;
    registrationNumber = "";
    parkingLocation = "";
    entryTimeStamp = "";
    exitTimeStamp = "";
    paymentIndicator = "";
  }*/
  // Created a parameterized constructor for the vehicle class
  Vehicle(char type_char, String registrationNumber_str, String parkingLocation_str, String entryTimeStamp_str, String exitTimeStamp_str, String paymentIndicator_str) {
    this->type = type_char;
    this->registrationNumber = registrationNumber_str;
    this->parkingLocation = parkingLocation_str;
    this->entryTimeStamp = entryTimeStamp_str;
    this->exitTimeStamp = exitTimeStamp_str;
    this->paymentIndicator = paymentIndicator_str;
  }
};

//create array to store instances of the vehicle
const int maxVehicles = 20;         // Maximum number of vehicles in the list
Vehicle* vehicleList[maxVehicles];  // Array to store the list of vehicles
int numVehicles = 0;

//function to display information on vehicles already stored in arduino
void displayVehicleInfo() {
  if (numVehicles > 0) {
    if (change == true) {
      lcd.clear();
      lcd.setCursor(0, 0);
      // print the custom char to the lcd
      lcd.write((uint8_t)0);
      lcd.print(vehicleList[index]->registrationNumber);
      lcd.print(" ");
      lcd.setCursor(9,0);
      if(vehicleList[index]->parkingLocation.length() > 7)
      {
        
        scrolling(vehicleList[index]->parkingLocation);

      }
      else{
        lcd.print(vehicleList[index]->parkingLocation);
      }

      lcd.setCursor(0, 1);
      lcd.write((uint8_t)1);

      lcd.print(vehicleList[index]->type);
      lcd.print(" ");
      lcd.print(vehicleList[index]->paymentIndicator);
      lcd.print(" ");
      lcd.print(vehicleList[index]->entryTimeStamp);
      lcd.print(" ");
      lcd.print(vehicleList[index]->exitTimeStamp);

      if (vehicleList[index]->paymentIndicator == "NPD") {
        lcd.setBacklight(3);  // Yellow backlight
      } else {
        lcd.setBacklight(2);  // Turn off backlight if not NPD
      }
      change = false;
    };

  } else {
    lcd.clear();
  }
  //boolean to check if index changes print from 0,0 again else
}
//fucntion to return current time, does not work
void updateTime() {
  // Update time in the Timer interrupt
  if (second() == 0) {
    // Additional time-related updates if needed
  }
}

String setTimeStamp() {
  // Return time in HHMM format with leading zeros
  return ((hour() < 10) ? "0" : "") + String(hour()) + ((minute() < 10) ? "0" : "") + String(minute());
}

void setup() {

  Serial.begin(9600);
  lcd.begin(16, 2);
  setTime(0, 0, 0, 1, 1, 2023);  // Set the initial time to 00:00 01/01/2023
  Timer1.initialize(1000000);    // Timer1 every second
  Timer1.attachInterrupt(updateTime);
  // for(int i = 0; i < maxVehicles; i++){
  // vehicleList[i] = nullptr;
  // }

  // create a new custom character
  lcd.createChar(0, customCharUp);
  lcd.createChar(1, customCharDown);

  // creating objects of the vehicle class to be displayed in the main pahse

  //addVehicle('V', "GI29WLN", "RoseLane.", "0003", "0000", "PD");
}
// Function to add a new vehicle to the list
void addVehicle(char type, String registrationNumber, String parkingLocation, String entryTimeStamp, String exitTimeStamp, String paymentIndicator) {
  if (numVehicles < maxVehicles) {
    // Create a new Vehicle object and add it to the list
    vehicleList[numVehicles] = new Vehicle(type, registrationNumber, parkingLocation, entryTimeStamp, exitTimeStamp, paymentIndicator);
    //vehicleList[numVehicles] = newVehicle;
    // Increment the number of vehicles in the list
    numVehicles++;
  } else {
    Serial.print("ERROR: Max vehicles");
    Serial.print("ERROR: Delete some.");  // Display error message
  }
}
//fucntion to delete an element from the vehicleslist when the R protocol is selected
//then it shifts all elements in list to the left
void deleteAtIndex(int indexToDelete, int listSize) {
  if (indexToDelete >= 0 && indexToDelete < listSize) {
    for (int n = indexToDelete; i < listSize - 1; ++n) {
      free(vehicleList[n]);
      vehicleList[n] = vehicleList[n + 1];
    }
    --listSize;
  }
}
void scrolling(String p_location){
  int a;
  if((del_Char +1000) < millis()){
    lcd.setCursor(9,0);
    lcd.print("                  ");
    lcd.setCursor(9,0);
    lcd.print(p_location.substring(i, p_location.length()));
    del_Char = millis();
    a++;
  }
  if(a == p_location.length()){
    a =0;
  }
}

// Function to validate parking location string
bool isValidParkingLocation(String p_location) {
  for (char c : p_location) {
    if (!(isAlphaNumeric(c) || c == '.')) {
      return false;
    }
  }
  return true;
}



void loop() {
  switch (state) {
    case synchronisation:
      {
        lcd.setBacklight(5);
        Serial.print("Q");
        delay(1000);

        if (Serial.available() > 0) {
          char input = Serial.read();
          if (input == 'X') {
            Serial.println("BASIC");
            lcd.setBacklight(7);
            state = main;
          }
        }
      }
      break;
    case main:
      {
        //change = false;
        uint8_t buttons = lcd.readButtons();
        if (selectButtonPressed == false) {
          displayVehicleInfo();

          if (buttons & BUTTON_UP) {
            if (index > 0) {
              index--;
              change = true;
            }
          }
          if (buttons & BUTTON_DOWN) {
            if (index < numVehicles - 1) {
              index++;
              change = true;
            }
          }

          if (index == 0) {
            lcd.setCursor(0, 0);
            lcd.print(" ");
            //change = false;
          }
          if (index == numVehicles - 1) {
            lcd.setCursor(0, 1);
            lcd.print(" ");
            //change = false;
          }
        }

        if (buttons & BUTTON_SELECT) {
          if (startPressed + 1000 < millis()) {
            // Clear the screen
            if (selectButtonPressed == false) {
              lcd.clear();
            }

            startPressed = millis();
            // Set the backlight to purple
            lcd.setBacklight(5);
            // Display student ID
            lcd.setCursor(0, 0);
            lcd.print("F321167");
            lcd.setCursor(0, 1);
            lcd.print(freeMemory());  //student ID
            selectButtonPressed = true;
          }
        } else {
          if (selectButtonPressed == true) {
            lcd.clear();
            selectButtonPressed = false;
            change = true;
          }
        }
        if (Serial.available() > 0) {
          String input = Serial.readString();  // Read the whole string
                                               // input.Strip();
          char protocol = input[0];
          //Serial.println(protocol);
          if (input[1] != '-' || input.indexOf(' ') != -1) {  //check for white spaces
            protocol = 'Z';                                   //set it to a char thats not a protocol so it goes to the dfaul case in the switch statement
            Serial.println(protocol);
          }
          String regNum = input.substring(2, 9);  // Use substring instead of substr
          int endIndex = input.length();
          int count = 0;

          switch (protocol) {
            case 'S':
              count = 0;  // always set to 0 at the start of each operation
              for (int i = 0; i < numVehicles; i++) {
                if (vehicleList[i]->registrationNumber == regNum) {
                  count++;
                  String storedStatus = vehicleList[i]->paymentIndicator;
                  String enteredStatus = input.substring(10, endIndex - 1);
                  //Serial.println(enteredStatus);
                  if (enteredStatus == "PD" || enteredStatus == "NPD") {  //code to ensure payment staus entered is not invalid

                    if ((vehicleList[i]->paymentIndicator == "PD") && (input.substring(10, endIndex) == "NPD")) {
                      vehicleList[i]->exitTimeStamp = " ";
                      vehicleList[i]->entryTimeStamp = setTimeStamp();
                      lcd.println(vehicleList[i]->entryTimeStamp);
                      Serial.println("DONE!");
                    }
                    // code to change status from unpaid to paid
                    else if (storedStatus == "NPD" && enteredStatus == "PD") {

                      vehicleList[i]->paymentIndicator = "PD";
                      Serial.println("DONE!");
                    }

                    else
                    //(vehicleList[i]->paymentIndicator == input.substring(10, endIndex))
                    {
                      Serial.println("ERROR: Payment status is the same!");
                    }
                  } else {
                    Serial.println("Invalid payment status!");
                  }
                }
              }
              if (count == 0)  // if no vehicle with that regnumber exists its an error
              {
                Serial.println("ERROR:Vehicles does not exist");
              }
              break;
            case 'T':
              count = 0;
              for (int i = 0; i < numVehicles; i++) {
                if (vehicleList[i]->registrationNumber == regNum) {
                  count++;  //checks if any vehicle with the reg number exists already
                  if (vehicleList[i]->type == input[10]) {
                    Serial.println("ERROR:Same type, cannot be modified");
                  }
                  if (vehicleList[i]->type != input[10])  //check if vehicle type being entered is different to the one already stored
                  {
                    if (vehicleList[i]->paymentIndicator == "NPD")  //check if payment has been made
                    {
                      Serial.println("ERROR:No payment made");
                    } else {
                      vehicleList[i]->type = input[10];  //modify the vehicle type
                      Serial.println("DONE!");
                    }
                  }
                }
              }
              break;

            case 'L':  //case for a location change
              count = 0;
              for (int i = 0; i < numVehicles; i++) {
                if (vehicleList[i]->registrationNumber == regNum) {
                  count++;  //checks if any vehicle with the reg number exists already
                  if (vehicleList[i]->parkingLocation == input.substring(10, endIndex)) {
                    Serial.println("ERROR: Same location, cannot be modified");
                    break;
                  }
                  if (vehicleList[i]->parkingLocation != input.substring(10, endIndex))  //check if vehicle type being entered is different to the one already stored
                  {
                    if (vehicleList[i]->paymentIndicator == "NPD")  //check if payment has been made
                    {
                      Serial.println("ERROR:No payment made");
                      break;
                    } else {
                      vehicleList[i]->parkingLocation = input.substring(10, endIndex);  //modify the vehicle location
                      Serial.println("DONE!");
                      break;
                    }
                  }
                }
              }
              if (count == 0)  //if no vehicle with that regnumber exists its an error
              {
                Serial.println("ERROR:Vehicle does not exist");
              }
              break;
            case 'R':  //REMOVE EXISTING VEHICLE
              count = 0;
              for (int i = 0; i < numVehicles; i++) {

                if (vehicleList[i]->registrationNumber == regNum) {
                  if (vehicleList[i]->paymentIndicator == "PD") {  //If payment has been made
                    deleteAtIndex(i, numVehicles);
                    Serial.println("DONE!");
                    count++;
                    break;
                  } else {
                    Serial.println("ERROR:No payment made");
                  }
                }
              }
              if (count == 0)  //if no vehicle with that regnumber exists its an error
              {
                Serial.println("ERROR:Vehicles does not exist");
              }
              break;
            case 'A':
              //code to check if parking location is longer than 11 characters

              if (input.length() >= 13) {                         // you cant do substring on a input that doesnt have the correct length
                String location = input.substring(12, endIndex);  // write code to Check if parking location contains invalid characters


                //check length meets standards
                if (location.length()-1 > 11) {
                  Serial.println("ERROR: Parking location string longer than 11 characters");
                } 
                //else if (!isValidParkingLocation(location)) {
                //   Serial.println("ERROR: Invalid parking location format");
                //   return;
                 else {
                  //if the location length is suitable process the information
                  for (int i = 0; i < numVehicles; i++) {
                    if (vehicleList[i]->registrationNumber == regNum) {
                      count++;  //checks if any vehicle with the reg number existed
                      if (vehicleList[i]->paymentIndicator == "PD") {
                        vehicleList[i]->type = input[10];
                        vehicleList[i]->parkingLocation = location;
                      } else {
                        Serial.println("ERROR:No payment made");
                      }
                    }
                  }
                  if (count == 0)  //if vehicle doesnt exist add to vehicle List
                  {
                    //if statement to check if vehicle has no type, produce error
                    addVehicle(input[10], regNum, input.substring(12, endIndex), setTimeStamp(), " ", "NPD");
                    Serial.println("DONE!");
                    change = true;
                    displayVehicleInfo();
                  }
                }
              } else {
                Serial.println("ERROR: Invalid format");
              }
              break;
            default:
              Serial.println("Debug: Inside default case");
              Serial.println("ERROR:Invalid format!");
              break;
          }
        }
      }
      break;
  }
}

