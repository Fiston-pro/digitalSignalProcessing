#include <Ethernet.h>
#include <SPI.h>
#include <OneWire.h>

// Ethernet settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x51 };  // Use your own MAC address
IPAddress ip(10, 23, 18, 251);  // IP address for the Arduino
EthernetServer server(80);  // Port 80 is default for HTTP

// OneWire settings
OneWire ds(2);  // Data wire connected to pin 2

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();
}

void loop() {
  EthernetClient client = server.available();

  if (client) {
    Serial.println("Wassup");
    if (client.connected()) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println("<!DOCTYPE HTML>");
      client.println("<html>");
      client.println("<head><title>Check out your Temperature</title></head>");
      client.println("<body>");

      // Read temperature
      float temperatureCelsius = readTemperature();
      client.print("<b>Temperature: </b>");
      client.print(temperatureCelsius, 2); // Display temperature with 2 decimal places
      client.println(" Celsius<br>");

      // Heating control
      if (temperatureCelsius < 28.0) {
        client.println("Heating: On<br>");
      } else {
        client.println("Heating: Off<br>");
      }

      // Cooling control
      if (temperatureCelsius > 30.0) {
        client.println("Cooling: On<br>");
      } else {
        client.println("Cooling: Off<br>");
      }

      client.println("</body>");
      client.println("</html>");
    }
    delay(1);
    client.stop();
    Serial.println("Client disconnected");
  }
}

float readTemperature() {
  byte data[12];
  byte addr[8];
 
  if (!ds.search(addr)) {
    ds.reset_search();
    return -1000; // Error value
  }
 
  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return -1000; // Error value
  }
 
  if (addr[0] != 0x10 && addr[0] != 0x28 && addr[0] != 0x22) {
    Serial.println("Device is not a DS18x20 family device.");
    return -1000; // Error value
  }
 
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1); // start conversion, with parasite power on at the end
  delay(1000); // Wait for conversion to complete
 
  ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad
 
  for (byte i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
 
  int16_t raw = (data[1] << 8) | data[0];
  float temperature = (float)raw / 16.0;
 
  return temperature;
}
