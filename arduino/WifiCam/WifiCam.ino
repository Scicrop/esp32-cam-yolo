#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>
#include <EEPROM.h>

bool testWifi(int);
void launchWeb(void);
void setupAP(void);

WebServer server(80); //server on port 80

static auto loRes = esp32cam::Resolution::find(320, 240); //low resolution
static auto hiRes = esp32cam::Resolution::find(800, 600); //high resolution
//static auto hiRes = esp32cam::Resolution::find(640, 480); //high resolution (for fps rates) (IP CAM APP)

int statusCode;
String content;
String select_input;
String st;
int buttonState = 0;
const int ledPin = 4;
const int wifiBtn = 5;
int reset = 0;
int counter = 0;
int ap_mode = 0;

void
serveJpg() //capture image .jpg
{
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("Capture Fail");
    server.send(503, "", "");
    return;
  }
  //Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(), static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);  //and send to a client (in this case it will be python)
}

void
handleJpgLo()  //allows to send low resolution image
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}

void
handleJpgHi() //allows to send high resolution image
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}

bool testWifi(int c){
  Serial.println("Waiting for WiFi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.println("*");
    c++;
  }
  Serial.println("Connection timed out, opening AP or Hotspot");
  return false;
}

void launchWeb(){
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void){
  ap_mode == 1;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan completed");
  if (n == 0)Serial.println("No WiFi Networks found");
  else{
    Serial.print(n);
    Serial.println(" Networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");

  select_input = "<select name='ssid'>\n";
  st = "<ol>";
  for (int i = 0; i < n; ++i) {
    // Print SSID and RSSI for each network found
    select_input += "<option>";
    st += "<li>";
    st += WiFi.SSID(i);
    select_input += WiFi.SSID(i);
    st += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*";
    st += "</li>";
    select_input += "</option>\n";

  }
  st += "</ol>";
  select_input += "</select>";
  delay(100);
  WiFi.softAP("SCICROP", "");
  Serial.println("Initializing_Wifi_accesspoint");
  launchWeb();
  Serial.println("over");
}

void createWebServer(){
 {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html><body><center><div style='width: 400px;'><h1>SciCrop WiFi Connectivity Setup</h1>";
      content += "<form action=\"/scan\" method=\"POST\"></form>";
      content += st;
      content += "<form method='get' action='setting'>\n<p><label style='font-size: 200%;'>Rede: </label>";
      content += select_input;
      content +="</p><p><label style='font-size: 200%;'>Senha: </label><input name='pass' length=64></p><p>\n<input type='submit' style='font-size: 200%;'></p></form>";
      content += "</div></center></body></html>";
      server.send(200, "text/html", content);

    });

    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    content = "<!DOCTYPE HTML>\r\n<html><body><center><h1>Device Configured! Device will reboot in 10 seconds.</h1></center></html>";
    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i){
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }

        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i){
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();


        statusCode = 200;
        server.send(statusCode, "text/html", content);
        ESP.restart();

      }else{
        statusCode = 404;
        Serial.println("Sending 404");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(statusCode, "text/html", content);
      }

    });

  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println('starting');
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(wifiBtn, INPUT);
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }

// BEGIN AUTO WIFI SETUP

  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  EEPROM.begin(512); //Initializing EEPROM
  delay(10);
  Serial.println("WIFI SETUP Startup");

  // Read eeprom for ssid and password
  Serial.println("Reading EEPROM ssid");

  String esid;
  int val = 0;
  int reseted = 20;
  for (int i = 0; i < 32; ++i){
    val = EEPROM.read(i);
    esid += char(val);
    if(val != 0) reseted = 0;
  }

  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i){
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);

  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi(reseted)){
    Serial.println("Succesfully Connected!!!");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/cam-lo.jpg");//to connect IP low res
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/cam-hi.jpg");//to connect high res IP
    server.on("/cam-lo.jpg",handleJpgLo);//send to the server
    server.on("/cam-hi.jpg", handleJpgHi);
    server.begin();
    while(counter < 3){
      digitalWrite(ledPin, HIGH);
      delay(300);
      digitalWrite(ledPin, LOW);
      delay(300);
      counter++;
    }
    counter=0;
    return;
  }else{
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();// Setup accesspoint or HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");
/*
  while(counter < 10){
    digitalWrite(ledPin, HIGH);
    delay(30);
    digitalWrite(ledPin, LOW);
    delay(1000);
    counter++;
  }
  counter = 0;
*/  
  while ((WiFi.status() != WL_CONNECTED)){
    delay(100);
    server.handleClient();
  }
  // END AUTO WIFI SETUP
}

void loop(){
  digitalWrite(ledPin, LOW);
  while(counter < 8){
    buttonState = digitalRead(wifiBtn);
    Serial.println(buttonState);
    if (buttonState == HIGH) {
      buttonState = LOW;
      reset++;
      if(reset >= 5) break;
      Serial.println("high.");
    } else {
      Serial.println("low.");
    }
    delay(1000);
    Serial.print("Counter: ");
    Serial.println(counter);
    counter++;

  }

  if(reset >= 5){
    digitalWrite(ledPin, HIGH);
    Serial.println("clearing eeprom");
    for (int i = 0; i < 96; ++i) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
    ESP.restart();
  }else{
    server.handleClient();
    //digitalWrite(ledPin, LOW);
  }
}