/*
Garage Door Opener
By Ferran Piñana aka Spyd
License: Open source, please contribute if you modify this code, please retain credits, if you use this please leave comments and impressions.
https://github.com/Spyd77/GorDO
*/

#include <SoftwareSerial.h>
#include <EEPROM.h>

//Arduino Mega ports
/*const int SIM900_RX = 52;
const int SIM900_TX = 53;
const int Relay_TX = 51;*/

//Arduino One ports
const int SIM900_RX = 7;
const int SIM900_TX = 8;
const int Relay_TX = 12;

const int PhoneNumberSize = 16;
const int PhoneNameSize = 47;
const int PhoneBookSize = 64;

SoftwareSerial SIM900(SIM900_RX, SIM900_TX); //Arduino Mega
//SoftwareSerial SIM900(7, 8); //Arduino UNO

String commandw(String command) {
  int lc=100;
  String response;
  char incoming_char=0;
  
  if (command != "") {
    Serial.println("Sending command: " + command);
    SIM900.println(command);
    Serial.println("Sent. Waiting response");
  } else {
    Serial.println("Receiving data");
  }
  do {
    delay(100);
    lc--;
    if (SIM900.available()) {
      do {
        incoming_char = SIM900.read();
        response += incoming_char;
      } while (SIM900.available());
      lc = 0;
    }
  } while ( lc > 1 );
  response.trim();
  return response;
}

void relay(int rn, bool o) {
  if (o) {
    digitalWrite(Relay_TX+rn-1, LOW);
  } else {
    digitalWrite(Relay_TX+rn-1, HIGH);
  }
}

void pushrelay(int rn) {
  relay(rn, true);
  delay(500);
  relay(rn, false);
}

void initRelays() {
  pinMode(Relay_TX, OUTPUT);
  relay(1,false);
/*  delay(1000);
  relay(1,true);*/
}

String getCaller(String response) {
  /*
  RING

  +CLIP: "679495539",129,"",,"",0
  */
  int sp;
  String caller="";
  int quotes=0;
  char actualchar;
  
  sp = response.indexOf("+CLIP",0);
  if (sp != -1) {
    do {
      actualchar = response.charAt(sp);
      if (actualchar == '"') {
        quotes++;
      } else {
        if (quotes == 1) {
          caller += actualchar;
        }
      }
      sp++;
    } while ((quotes < 2) && (sp < response.length()));
  }
  return caller;
}

String authorizedCaller(String caller) {
  int f=0;
  char phoneNumber[PhoneNumberSize];
  char phoneName[PhoneNameSize];
  String gotNumber;
  int found=-1;

  for (f=0; f<PhoneBookSize; f++) {
    EEPROM.get( f*(PhoneNumberSize+PhoneNameSize), phoneNumber );
    gotNumber = phoneNumber;
    if (gotNumber == caller) {
      found = f;
      break;
    }
  }
  if (found != -1) {
    EEPROM.get( (found*(PhoneNumberSize+PhoneNameSize))+PhoneNumberSize, phoneName );
    return phoneName;
  } else {
    return "";
  }
}

void ring(String response) {
  String caller;
  String who;
  
  caller = getCaller(response);
  Serial.println("Caller: "+caller);
  who = authorizedCaller(caller);
  if (who!="") {
    Serial.println("You can pass, "+who+".");
    pushrelay(1);
  } else {
    Serial.println("I don't know you");
  }
  Serial.println(commandw("ATH"));
}

void SMS(String response) {
  
}

String phoneBook() {
  String s="";
  int f=0;
  int g;
  byte temp;
  char phoneNumber[PhoneNumberSize];
  char phoneName[PhoneNameSize];
  char phoneAdmin;
  String gotNumber;
  
  do {
    EEPROM.get( f*(PhoneNumberSize+PhoneNameSize+1), phoneNumber );
    EEPROM.get( (f*(PhoneNumberSize+PhoneNameSize+1))+PhoneNumberSize, phoneName );
    EEPROM.get( (f*(PhoneNumberSize+PhoneNameSize+1))+PhoneNumberSize+PhoneNameSize, phoneAdmin );
    gotNumber = "";
    for (g = 0; g < PhoneNumberSize; g++) {
      temp = phoneNumber[g];
      if ((temp!=0) && (temp!=255)) {
        gotNumber = gotNumber + phoneNumber[g];
      } else {
        break;
      }
    }
    if (gotNumber != "") {
      s = s + phoneNumber + " (" + phoneName + ") (" + phoneAdmin + ")\n";
    }
    f++;
  } while (f < PhoneBookSize);
  return s;
}

String addPhone(String command, bool admin) {
  String strNumber="";
  String strName="";
  String result;
  char phoneNumber[PhoneNumberSize];
  char phoneName[PhoneNameSize];
  char phoneAdmin;
  int f;
  int n=0;
  int EmptyEntry=-1;
  byte test;
  for (f=2; f<command.length(); f++) {
    if ((n==0) && (command[f]==' ')) {
      n=1;
    } else {
      if (n==0) {
        strNumber = strNumber + command[f];
      } else {
        strName = strName + command[f];
      }
    }
  }
  if (strNumber != "") {
    for (f=0; f<PhoneBookSize; f++) {
      EEPROM.get( f*(PhoneNumberSize+PhoneNameSize+1), phoneNumber );
      test = phoneNumber[0];
      if ((test == 0) || (test == 255)) {
        EmptyEntry = f;
        break;
      }  
    }
  }
  if (EmptyEntry != -1) {
    strNumber.toCharArray(phoneNumber, PhoneNumberSize);
    strName.toCharArray(phoneName, PhoneNameSize);
    if (admin) {
      phoneAdmin='Y';
    } else {
      phoneAdmin='N';
    }
    EEPROM.put( f*(PhoneNumberSize+PhoneNameSize+1), phoneNumber );
    EEPROM.put( (f*(PhoneNumberSize+PhoneNameSize+1))+PhoneNumberSize, phoneName );
    EEPROM.put( (f*(PhoneNumberSize+PhoneNameSize+1))+PhoneNumberSize+PhoneNameSize, phoneAdmin );
    result = "New entry: ";
    result = result + EmptyEntry;
  } else {
    result = "Error: Phonebook full, max entries: ";
    result = result + PhoneBookSize;
  }
  return result;
}

String delPhone(String command) {
  String strNumber="";
  String result;
  String gotNumber;
  char phoneNumber[PhoneNumberSize];
  int f;
  int g;
  int n=0;
  byte test;
  int found=-1;

  for (f=2; f<command.length(); f++) {
    if ((n==0) && (command[f]==' ')) {
      n=1;
    } else {
      if (n==0) {
        strNumber = strNumber + command[f];
      } else {
        break;
      }
    }
  }
  if (strNumber != "") {
    for (f=0; f<PhoneBookSize; f++) {
      EEPROM.get( f*(PhoneNumberSize+PhoneNameSize+1), phoneNumber );
      gotNumber = phoneNumber;
      if (gotNumber == strNumber) {
        found = f;
        break;
      }
    }
    if (found != -1) {
      phoneNumber[0]=0;
      EEPROM.put( found*(PhoneNumberSize+PhoneNameSize+1), phoneNumber );
      result = "Found and deleted on entry: ";
      result = result + found;
    } else {
      result = "Phone number not found";
    }
  } else {
    result = "no phone number given";
  }
  return result;
}

String cCommand(String command) {
  int f;
  String sendCommand="";

  for (f=8; f<command.length(); f++) {
    sendCommand = sendCommand + command[f];
  }
  return commandw(sendCommand);
}

void initPhone() {
  SIM900.begin(19200);
  Serial.println(commandw("ATZ"));
  //Serial.println(commandw("ATE0"));
  //Serial.println(commandw("AT+CLIP=?"));
  //Serial.println(commandw("AT+CLIP?"));
  Serial.println(commandw("AT+CLIP=1"));
}

void setup() {
  Serial.begin(19200);
  Serial.println("Setup begins");
  initPhone();
  initRelays();
  Serial.println("Setup end");
}

void loop() {
  String response;
  String command;
  bool knownMessage;
  bool knownCommand;
  byte firstChar;
  if (SIM900.available()) {
    response = commandw("");
    knownMessage = false;
    firstChar = response[0];
    if (firstChar==255) {
      knownMessage = true;
      initPhone();
    }
    if (response.startsWith("RING")) {
      knownMessage = true;
      ring(response);
    }
    if (response.startsWith("+CMTI:")) {
      knownMessage = true;
      SMS(response);
    }
    if (response == "OK") {
      knownMessage = true;
    }
    if (!knownMessage) {
      Serial.println("Unknown message:");
      Serial.println(response);
    }
  }
  if (Serial.available() > 0) {
    knownCommand = false;
    Serial.println("Console command:");
    command = Serial.readString();
    if (command.startsWith("list")) {
      knownCommand = true;
      Serial.println(phoneBook());
    }
    if (command.startsWith("a ")) {
      knownCommand = true;
      Serial.println(addPhone(command,false));
    }
    if (command.startsWith("A ")) {
      knownCommand = true;
      Serial.println(addPhone(command,true));
    }
    if (command.startsWith("d ")) {
      knownCommand = true;
      Serial.println(delPhone(command));
    }
    if (command=="messages") {
      knownCommand = true;
      Serial.println(commandw("AT+CMGL=\"ALL\""));
    }
    if (command.startsWith("command")) {
      knownCommand = true;
      Serial.println(cCommand(command));
    }
    if (command=="?") {
      knownCommand = true;
      Serial.println("Avaible commands:");
      Serial.println("list = shows phonebook: number (name) (admin status) to serial console");
      Serial.println("a phone name = inserts into the phonebook the \"phone\" number for \"name\" as regular user");
      Serial.println("A phone name = inserts into the phonebook the \"phone\" number for \"name\" as administrator");
      Serial.println("d phone = deletes phone number \"phone\" from the phonebook");
      Serial.println("messages = dumps all messages to serial console");
      Serial.println("command cmd = sends AT command to GSM module and dumps response to serial console");
    }
    
    if (!knownCommand) {
      Serial.println("Unknown command:");
      Serial.println(command);
    }
  }
}
