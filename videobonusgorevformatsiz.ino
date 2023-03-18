#include <sd_defines.h>
#include <SD_MMC.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>

//Static IP address
IPAddress local_IP(192, 168, 0, 13);

//Gateway IP address
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(0, 0, 0, 0); 
 
const char* ssid = "grizu263"; 
const char* password = "grizu263"; 

WebServer server1(80);
WebServer server2(81);

void handleFileUpload() {
  static File uploadFile;
  HTTPUpload& upload = server1.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/" + upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    Serial.printf("Upload start: %s\n", filename.c_str());
    uploadFile = SD_MMC.open(filename, FILE_WRITE);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    size_t bytesWritten = uploadFile.write(upload.buf, upload.currentSize);
    Serial.printf("Upload write: %d bytes\n", bytesWritten);
  } else if (upload.status == UPLOAD_FILE_END) {
    uploadFile.close();
    Serial.printf("Upload end, size %d\n", upload.totalSize);
  }
}

void handleRoot() {
  server1.send(200, "text/html", "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='file'><input type='submit'></form>");
}

void handleFileDownload() {
  File file = SD_MMC.open("/" + server2.arg("filename"), FILE_READ);
  
  if (file) {
    server2.streamFile(file, "application/octet-stream");
    file.close();
  } else {
    server2.send(404, "text/plain", "File not found");
  }
}

void setup() {
  Serial.begin(115200);

    if(!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
      Serial.println("STA Failed to configure");
    }
    
    WiFi.begin(ssid, password);
    
    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
    
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  Serial.println("SD Card mounted");
  
  server1.on("/", HTTP_GET, handleRoot);
  server1.on("/upload", HTTP_POST, []() {
    server1.send(200, "text/plain", "File uploaded successfully");
  }, handleFileUpload);

  server1.begin();
  Serial.println("HTTP server1 started");

  server2.on("/download", HTTP_GET, handleFileDownload);
  server2.begin();
  Serial.println("HTTP server2 started");
}

void loop() {
  server1.handleClient();
  server2.handleClient();
}
