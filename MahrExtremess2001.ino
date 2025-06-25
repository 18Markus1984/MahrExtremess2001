// Use Serial1 for UART communication
//RES1,RES2,RES3 + \r --> Range
//TOL?\r --> Toleranz
//SET?\r --> Status
//?\r --> Messwert
//RST\r --> Reset und ABS wird deaktiviert
//BAT?\r --> Batteriestatus
//MAX\r --> Display Max/Min
//MIN\r --> Display Max/Min
//OFF\r --> herunterfahren
//ABS\r --> Absolute

#define NUM_VALUES 100

//Webserver
#include <WiFi.h> //ESP32
#include <ESPForm.h>

#include "html.h"

//Mit CoolTerm lassen sich die Ausgaben der Serielen Schnittstelle auslesen
HardwareSerial mySerial(1);
bool dataRecived = false;
float messStartTime = 0;

//Variablen für Progress
bool startstopbtn = false;
int modi = 0;


float lastValues[NUM_VALUES];
int valueIndex = 0;
bool bufferFilled = false;
float stableThreshold = 0.001;  // z. B. ±0.05 mm Toleranz für stabile Werte
int numberOfValues = 40;


//Your WiFi SSID and Password
unsigned long prevMillis = 0;
unsigned long serverTimeout = 2 * 60 * 1000;

//The AP
String apSSID = "MahrExtramess2001";
String apPSW = "12345678";

//wifi and webserver realted functions
bool startWiFi();                                                         //starts the Wifi protocol
void setupESPForm();                                                      //Initialised all the EventListener for the comminication between Esp and HTML
void formElementEventCallback(ESPFormClass::HTMLElementItem element);     //function that gets called if any of the EventListener get triggered. Used to get the different button presses and value changes
void serverTimeoutCallback();   

void setup()
{
  Serial.begin(115200);

  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true);
  WiFi.persistent(false);

  if (!startWiFi()) {
    setupESPForm();
    Serial.println("MAIN:  Start server");
    ESPForm.startServer();
  }

  Serial.println("Initialising Serial Connection....");
  mySerial.begin(4800, SERIAL_7E2, 21, 20);  // UART setup RX/TX
  Serial.println("ESP32 UART MahrConnect Extremess 2001");
}


void loop() {
  //If a client existed
  if (ESPForm.getClientCount() > 0)
  {

    if (millis() - prevMillis > 1000)
    {
      prevMillis = millis();
      //The event listener for text2 is not set because we don't want to listen to its value changes
      ESPForm.setElementContent("text2", String(millis()));
    }
  }
  if(startstopbtn){
    if(modi == 0) {
      wiederholgenauigkeit();
    } else if (modi == 1) {
      ueberschwingverhalten();
    }
  }
}

//starts the Wifi protocol
bool startWiFi(){
  //WiFi data is ready then start connction
  WiFi.mode(WIFI_AP);
  Serial.print("MAIN:  Connecting to Wi-Fi..");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("MAIN:  Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
  }
  else {
    Serial.println();
    Serial.println("MAIN:  WiFi connection failed!");
    return false;
  }
  return true;
}

//Initialised all the EventListener for the comminication between Esp and HTML
void setupESPForm(){
  Serial.println("MAIN:  Setup ESPForm");
  ESPForm.setAP(apSSID.c_str(), apPSW.c_str());
  //Prepare html contents (in html.h) for the web page rendering (only once)
  //Flash's uint8_t array, file name, size of array, gzip compression
  ESPForm.addFileData(index_html, "index.html");

  //Add html element event listener, id "text1" for onchange event
  
  ESPForm.addElementEventListener("modeSelectComBox", ESPFormClass::EVENT_ON_CHANGE);
  ESPForm.addElementEventListener("averageCount", ESPFormClass::EVENT_ON_CHANGE);
  ESPForm.addElementEventListener("tolerance", ESPFormClass::EVENT_ON_CHANGE);
  ESPForm.addElementEventListener("startStopBtn", ESPFormClass::EVENT_ON_CLICK);
  
  //Start ESPForm's Webserver
  ESPForm.begin(formElementEventCallback, serverTimeoutCallback, serverTimeout, true);

  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.println("=================================================");
  Serial.println("Use web browser and navigate to " + WiFi.localIP().toString());
  Serial.println("=================================================");
  Serial.println();
}


void serverTimeoutCallback()
{
  //If server timeout (no client connected within specific time)
  Serial.println("***********************************");
  Serial.println("Server Timeout");
  Serial.println("***********************************");
  Serial.println();
}

void formElementEventCallback(ESPFormClass::HTMLElementItem element)
{
  Serial.println();
  Serial.println("***********************************");
  Serial.println("id: " + element.id);
  Serial.println("value: " + element.value);
  Serial.println("type: " + element.type);
  Serial.println("event: " + ESPForm.getElementEventString(element.event));
  Serial.println("***********************************");
  Serial.println();

  if(element.id == "startStopBtn" && !startstopbtn){
    startstopbtn = true;
  }else if(element.id == "startStopBtn") {
    startstopbtn = false;
  }
  if(element.id == "modeSelectComBox" && modi != 1){
    modi = 1;
  }else if(element.id == "modeSelectComBox") {
    modi = 0;
  }
  if(element.id == "averageCount"){
    numberOfValues = element.value.toInt();
  }
  if(element.id == "tolerance"){
    stableThreshold = element.value.toFloat();
  }
}


void wiederholgenauigkeit() {
  // Anfrage senden
  mySerial.write("?\r");

  // Antwort einlesen
  String message = "";
  float startTime = millis();
  while (millis() - startTime < 300) {
    if (mySerial.available()) {
      char c = mySerial.read();
      message += c;
      if (c == '\r') {
        break;
      }
    }
  }

  // Gültige Nachricht verarbeiten
  if (message.length() > 0 && message != "ERR0\r" && !dataRecived) {
    message.trim();
    int index = message.indexOf("mm");
    if (index > 0) {
      String numberString = message.substring(0, index);
      numberString.replace(",", "."); // sicherstellen, dass '.' als Dezimaltrennzeichen erkannt wird
      float sensorValue = numberString.toFloat();

      // Neuen Wert in Puffer speichern
      lastValues[valueIndex] = sensorValue;
      valueIndex = (valueIndex + 1) % numberOfValues;
      if (valueIndex == 0) bufferFilled = true;

      // Wenn Puffer voll, Stabilität prüfen
      if (bufferFilled && isStable()) {
        float currentTime = millis() / 1000.0;
        if(messStartTime == 0){
          messStartTime = currentTime;
        }
        currentTime = currentTime - messStartTime;
        String formatedTime = String(currentTime, 3);
        formatedTime.replace(".", ",");

        Serial.print(formatedTime);
        Serial.print(";");

        message.replace(".", ","); // wieder deutsches Dezimaltrennzeichen
        Serial.println(message.substring(0, index));
        dataRecived = true;
      }
    }
  }
  if(message == "ERR0\r"){
    dataRecived = false;
  }
}

// Hilfsfunktion zur Stabilitätsprüfung
bool isStable() {
  float reference = lastValues[0];
  for (int i = 1; i < numberOfValues; i++) {
    if (abs(lastValues[i] - reference) > stableThreshold) {
      return false;
    }
  }
  return true;
}

void ueberschwingverhalten() {
  // Anfrage senden
  mySerial.write("?\r");  // oder einfach "?" wenn kein CR gefordert ist

  // Warte bis Antwort kommt (max. 500 ms)
  String message = "";
  float startTime = millis();
  while (millis() - startTime < 300) {
    //Fall Daten Verfügbar sind werden diese eingelsen bis ein CR("\r") gesendet wird. Dies beendet den Datensatz
    if (mySerial.available()) {
      char c = mySerial.read();
      message += c;
      if(c == '\r') {
        break;
      }
    }
  }
  //Falls die Nachricht Inhalt hat und kein Error ist wird die Nachricht mit der entsprechenden Sekunde ausgegeben 
  if(message == "ERR0\r" && dataRecived) {
    Serial.println(".");
  }
  if (message.length() > 0 && message != "ERR0\r") {
    float currentTime = millis()/1000.0;
    if(messStartTime == 0){
      messStartTime = currentTime;
    }
    currentTime = currentTime - messStartTime;
    String formatedTime = String(currentTime,3);
    formatedTime.replace(".", ",");
    Serial.print(formatedTime);
    //Trennzeichen für das erleichtere Speichern in einer Tabelle
    Serial.print(";");
    //Alle Leerzeichen werden entfernt
    message.trim();
    //Der index für die Teile des Strings nach mm werden ausgelesen, damit nur noch die Daten vorhanden sind. 
    //Da die Daten aber bei unterschiedlicher Auflösung unterschiedliche Längen haben ist es hier sinnvoll es über den Index zu lösen
    int index = message.indexOf("mm");
    message.replace(".", ",");
    Serial.println(message.substring(0, index));
    dataRecived = true;
  } 
  else {
    dataRecived = false;
  }
  //Das delay bestimmt die Abtastrate für die Messung. Bei einer Baudrate von 4800 sind im Idealfall ca. 434 Messungen mit 11 Bits für jeden Datensatz 
  //möglich, da aber ab 10ms schon die Tasten auf der Messuhr nicht mehr funktionieren, wurde sich für eine Abtastrate von 5ms entschieden. 
  //delay(5);
}
