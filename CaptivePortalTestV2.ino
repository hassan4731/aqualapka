#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
String macAdress = "";
String Email = "";
const char* AP_SSID = "ESP32-WiFi-Config";
const char* AP_PASS = "";
IPAddress apIP(192, 168, 0, 1);
DNSServer dnsServer;
WebServer server(80);
const byte DNS_PORT = 53;
String a;  // Mac adress will save in a
uint8_t newMACAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
String userid="";
HTTPClient https;
String responseHTML = ""
                      "<!DOCTYPE html><html><head><title>WiFi Config Portal</title></head>"
                      "<style>"
                      "body { background-color: #f1f1f1; font-family: Arial, Helvetica, sans-serif; }"
                      "form { background-color: white; border-radius: 5px; padding: 20px; width: 50%; margin: auto; margin-top: 50px; box-shadow: 0px 0px 10px #ccc; }"
                      "input[type=text], input[type=password] { width: 100%; padding: 12px 20px; margin: 8px 0; display: inline-block; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; }"
                      "input[type=submit] { background-color: #4CAF50; color: white; padding: 14px 20px; margin: 8px 0; border: none; border-radius: 4px; cursor: pointer; }"
                      "input[type=submit]:hover { background-color: #45a049; }"
                      "</style>"
                      "<body><h1 style='text-align: center;'>WiFi Config Portal</h1><form method='POST' action='/submit'>"
                      "<label for='ssid' style='font-size: 20px; display: block;'><b>SSID:</b></label>"
                      "<input type='text' id='ssid' name='ssid'>"
                      "<label for='password' style='font-size: 20px; display: block; margin-top: 20px;'><b>Password:</b></label>"
                      "<input type='password' id='password' name='password'>"
                      "<label for='timezone' style='font-size: 20px; display: block; margin-top: 20px;'><b>Timezone:</b></label>"
                      "<input type='text' id='timezone' name='timezone'>"
                      "<label for='devicename' style='font-size: 20px; display: block; margin-top: 20px;'><b>Device Name:</b></label>"
                      "<input type='text' id='devicename' name='devicename'>"
                      "<br><br><input type='submit' value='Submit'></form></body></html>";


void printConfigFile() {

  File configFile = SPIFFS.open("/config.txt", "r");
  if (!configFile) {
    Serial.println("Failed to open config file.");
    return;
  }

  String line;
  while (configFile.available()) {
    line = configFile.readStringUntil('\n');
    Serial.println(line);
  }

  configFile.close();
}

void loadParametersFromSPIFFS(String& timezone, String& deviceName, String& ssid, String& password) {
  if (SPIFFS.begin(true)) {
    Serial.println("SPIFFS mounted successfully");
    if (SPIFFS.exists("/config.txt")) {
      Serial.println("Loading parameters from SPIFFS");
      File configFile = SPIFFS.open("/config.txt", "r");
      if (configFile) {
        while (configFile.available()) {
          String line = configFile.readStringUntil('\n');
          line.trim();
          if (line.startsWith("timezone=")) {
            timezone = line.substring(9);
            Serial.print("Timezone: ");
            Serial.println(timezone);
          } else if (line.startsWith("device_name=")) {
            deviceName = line.substring(12);
            Serial.print("Device name: ");
            Serial.println(deviceName);
          } else if (line.startsWith("ssid=")) {
            ssid = line.substring(5);
            Serial.print("SSID: ");
            Serial.println(ssid);
          } else if (line.startsWith("password=")) {
            password = line.substring(9);
            Serial.print("Password: ");
            Serial.println(password);
          }
        }
        configFile.close();
      } else {
        Serial.println("Failed to read config file");
      }
    } else {
      Serial.println("Config file not found");
    }
  } else {
    Serial.println("Failed to mount SPIFFS");
  }
}

void saveConfig(String timezone, String deviceName, String ssid, String password) {
  File configFile = SPIFFS.open("/config.txt", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }
  configFile.print("ssid=");
  configFile.println(ssid);
  configFile.print("password=");
  configFile.println(password);
  configFile.print("timezone=");
  configFile.println(timezone);
  configFile.print("device_name=");
  configFile.println(deviceName);
  configFile.close();
  Serial.println("Config saved");
  Serial.println("\n\nTesting Config File\n");
  printConfigFile();
}

void setupWiFiAP() {
  Serial.println("Starting AP Mode");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
}

bool connectToWiFi(String ssid, String password) {
  if (ssid == "" || password == "") {
    Serial.println("SSID or password is missing. Please check your config file.");
    return false;
  }
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid.c_str(), password.c_str());
  // Wait for connection
  int count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 10) {
    delay(1000);
    Serial.print(".");
    count++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected");
    // Getting MAC after connecting with Wifi
    delay(500);
    Serial.print("ESP32 Board MAC Address:  ");
    a = WiFi.macAddress();
    Serial.println(a);

    return true;
  } else {
    Serial.println("Failed to connect to WiFi");
    return false;
  }
}

bool connectToWiFi() {
  String ssid, password, devicename, timezone;
  loadParametersFromSPIFFS(timezone, devicename, ssid, password);
  Serial.println("---------------------------------");
  Serial.println("Data From SPIFFS");
  Serial.println(ssid);
  Serial.println(password);
  Serial.println(devicename);
  Serial.println(password);
  if (ssid == "" || password == "") {
    Serial.println("SSID or password is missing. Please check your config file.");
    return false;
  }
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid.c_str(), password.c_str());
  // Wait for connection
  int count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 10) {
    delay(1000);
    Serial.print(".");
    count++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected");
    // Getting MAC after connecting with Wifi
    delay(500);
    Serial.print("ESP32 Board MAC Address:  ");
    a = WiFi.macAddress();
    Serial.println(a);
      File stringFile = SPIFFS.open("/myString.txt", "r");
      if (!stringFile) {
        Serial.println("Failed to open file for reading userID ");
        return false;
      }
      userid = stringFile.readString();
      stringFile.close();
      Serial.print("after reading the userid from file : ");
      Serial.println(userid);
    return true;
  } else {
    Serial.println("Failed to connect to WiFi");
    return false;
  }
}

void FunctionReceiveUser()
{ 
  // Get values from form submission
  if (server.arg("id") != "") {
    userid = server.arg("id");
    Serial.print("User ID: ");
    Serial.println(userid);
  File stringFile = SPIFFS.open("/myString.txt", "w");
  if (!stringFile) {
    Serial.println("Failed to open file for writing of UserID");
    return;
  }
  stringFile.println(userid);
  stringFile.close();

  }

}


void setupWebServer() {
  server.on("/", []() {
    server.sendHeader("Content-Type", "text/html");
    server.send(200, "text/html", responseHTML);
  });

  server.on("/submit", []() {
    server.send(200, "text/html", "<h1>Submitted!!!</h1>");
  });

  server.on("/success", []() {
    server.sendHeader("Content-Type", "text/html");
    server.send(200, "text/html", "<h1>Connected to WiFi!</h1>");
  });

  server.on("/receiveddata", []() {
    FunctionReceiveUser();
    //    server.sendHeader("Content-Type", "text/html");
    //    server.send(200, "text/html", "<h1>I am Receiving the data!</h1>");
  });
}

void setupDNSServer() {
  dnsServer.start(DNS_PORT, "*", apIP);
}

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
  }
  // Initialize WiFi in AP mode with the given SSID and password
  if (connectToWiFi() == false)
  {
    setupWiFiAP();
    // Start DNS server to redirect all traffic to the AP IP
    setupDNSServer();

    // Set up routes for web server
    setupWebServer();

    // Begin web server
    server.begin();
    //    Serial.println("WiFi Credentials are Wrong/Missing");
  }
  Serial.print("WiFi is Connected at this IP Address : ");
  Serial.println(WiFi.localIP());
}


void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  delay(3000);
//  String url = (String) "https://www.hsxperts.com/web-projects/aqualapka/wp-json/update_bowl_esp_mac/on-web?mac_address=" + (String)a + "&email=shawn@hsxperts.com";
//  Serial.println(url);
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("userid :     ");
    Serial.println(userid);
  //  https.begin(url.c_str());
  //  delay(50);
  //  https.addHeader("Content-Type", "application/x-www-form-urlencoded");
  //  int httpResponseCode = https.GET();
      
      delay(50);
      https.addHeader("Content-Type", "application/x-www-form-urlencoded");
      int httpResponseCode = https.GET();



    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      https.end();
    }
  }
}
