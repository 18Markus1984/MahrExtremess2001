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
//Mit CoolTerm lassen sich die Ausgaben der Serielen Schnittstelle auslesen
HardwareSerial mySerial(1);
bool dataRecived = false;
float pufferData[80];
float messStartTime = 0;


#define NUM_VALUES 40
float lastValues[NUM_VALUES];
int valueIndex = 0;
bool bufferFilled = false;

float stableThreshold = 0.001;  // z. B. ±0.05 mm Toleranz für stabile Werte

void setup() {
  Serial.begin(115200);
  mySerial.begin(4800, SERIAL_7E2, 21, 20);  // UART setup RX/TX
  Serial.println("ESP32 UART MahrConnect Extremess 2001");
}

void loop() {
  ueberschwingverhalten();
  //wiederholgenauigkeit();
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
      valueIndex = (valueIndex + 1) % NUM_VALUES;
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
  for (int i = 1; i < NUM_VALUES; i++) {
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
