/* 
LinkIt7697 (Line Notify)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-11-14 00:00
https://www.facebook.com/francefu
*/

const char* ssid     = "teacher";   //your network SSID
const char* password = "12345678";   //your network password
String token = "";

#include <LWiFi.h>

static const char rootCA[] = "-----BEGIN CERTIFICATE-----\r\n"
"MIIGYjCCBUqgAwIBAgIQCKNjEl2C+05Z7F44E4EjmzANBgkqhkiG9w0BAQsFADBG\r\n"
"MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRUwEwYDVQQLEwxTZXJ2ZXIg\r\n"
"Q0EgMUIxDzANBgNVBAMTBkFtYXpvbjAeFw0yMjA5MjcwMDAwMDBaFw0yMzA5MjYy\r\n"
"MzU5NTlaMB8xHTAbBgNVBAMTFHVzLWVhc3QtMS5zaWduaW4uYXdzMIIBIjANBgkq\r\n"
"hkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA36zuci6fRgK423h3dvLeZMdqwWi1I2p/\r\n"
"Q0qtCnvF6QpqDLkbWYWG/Mi9l/GU2ZihtsjjqUaqdjikq24GzgtYus9VXc/ys0T0\r\n"
"df0suzj9X8nOKrKsxXdTpmFRYLCDbSXZB7kzfcSVmujoE0LuenrBRVyJSqK9QDov\r\n"
"nZnIHtx8I3lUDT3TB/V2PiiZ5mB+ChILosz4sli6XZPe7utAWaa52EXgJeZzh1MC\r\n"
"+yeIhkjALU2AH2jxNx4qBLdXv0xO3aVVN2iElnS0LDMmE3VDepSgfakHxkGsus4P\r\n"
"snPOnWQUCvDUJLqQtlFL5knEgkE2br6NDyl8CKeklTKxu4yb2YOanQIDAQABo4ID\r\n"
"cTCCA20wHwYDVR0jBBgwFoAUWaRmBlKge5WSPKOUByeWdFv5PdAwHQYDVR0OBBYE\r\n"
"FBdfQCN2xO9E+y7gZnqzYCXsD3kfMIGiBgNVHREEgZowgZeCFHVzLWVhc3QtMS5z\r\n"
"aWduaW4uYXdzghYqLnVzLWVhc3QtMS5zaWduaW4uYXdzghVzaWduaW4uYXdzLmFt\r\n"
"YXpvbi5jb22CFyouc2lnbmluLmF3cy5hbWF6b24uY29tghh1cy1lYXN0LTEuYXBp\r\n"
"LnNpZ25pbi5hd3OCCnNpZ25pbi5hd3OCEXVwZGF0ZS5zaWduaW4uYXdzMA4GA1Ud\r\n"
"DwEB/wQEAwIFoDAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwPQYDVR0f\r\n"
"BDYwNDAyoDCgLoYsaHR0cDovL2NybC5zY2ExYi5hbWF6b250cnVzdC5jb20vc2Nh\r\n"
"MWItMS5jcmwwEwYDVR0gBAwwCjAIBgZngQwBAgEwdQYIKwYBBQUHAQEEaTBnMC0G\r\n"
"CCsGAQUFBzABhiFodHRwOi8vb2NzcC5zY2ExYi5hbWF6b250cnVzdC5jb20wNgYI\r\n"
"KwYBBQUHMAKGKmh0dHA6Ly9jcnQuc2NhMWIuYW1hem9udHJ1c3QuY29tL3NjYTFi\r\n"
"LmNydDAMBgNVHRMBAf8EAjAAMIIBfAYKKwYBBAHWeQIEAgSCAWwEggFoAWYAdQCt\r\n"
"9776fP8QyIudPZwePhhqtGcpXc+xDCTKhYY069yCigAAAYOAeWCVAAAEAwBGMEQC\r\n"
"IAHBJfmKhzInbQ7X7u4pMLnHo39gcmTcaM0XhtOkaGkzAiA/k4bypMI7hHd6TP2T\r\n"
"dx91To30QAhF6zvQ5I2Ukh6ppAB1ADXPGRu/sWxXvw+tTG1Cy7u2JyAmUeo/4Srv\r\n"
"qAPDO9ZMAAABg4B5YIkAAAQDAEYwRAIgIuyxrkVBHOc99Sq7O/PkuDTb9b6LX8Xc\r\n"
"wX3EVy88N0ACIARKnlRkzPIWNLWv4WNPfh7Q/PirKcRIDsP4KZKFf1EqAHYAs3N3\r\n"
"B+GEUPhjhtYFqdwRCUp5LbFnDAuH3PADDnk2pZoAAAGDgHlgyAAABAMARzBFAiBQ\r\n"
"XWYnvR0NeQlRlovTCJA9OFHa7Ol32PnYrMfQHdxFrwIhAKtQhH7YlMQSG2RS3y8v\r\n"
"VZZ0auzxRSNKsSfXgBmc3sOLMA0GCSqGSIb3DQEBCwUAA4IBAQCrrddUcqyht9zl\r\n"
"bTysiAzo1EWXTIsqBSKj+gW88E2wFJo6JL+kOFK08vZKv9e78BSYlCumO4qtKiRg\r\n"
"3dGv7CX6b10hjfX0mACq+bFN9j3Y/s27ZQ6qXmtjycA40cYmwCBJ+0LiRPjuCJji\r\n"
"fC3K/2gM0d5bTaeSFpNG64yn5eysfYBon34I7+XTy9Zg3yWe4Z5p1poP6iFDPgdF\r\n"
"1WK27i5TpQc3uw/O/zKgORMFor2dPexHWY99JKSv0vHEppKne622czLZFn7ro9M8\r\n"
"qL7rJvxwMLPky0tvX6XqrM4jXrOdr1uUbhusLRHxqPRjUftuXKUMU8KmSwLLqSmT\r\n"
"fjr4TyYy\r\n"
"-----END CERTIFICATE-----\r\n";

void setup()
{
    Serial.begin(9600);
    delay(10);
    //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

    WiFi.begin(ssid, password);

    delay(1000);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ((StartTime+10000) < millis()) break;
    } 
  
    if (WiFi.status() == WL_CONNECTED) {
      pinMode(LED_BUILTIN, OUTPUT);
      for (int i=0;i<5;i++) {
        digitalWrite(LED_BUILTIN,HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN,LOW);
        delay(100);
      }

    } else {
      pinMode(LED_BUILTIN, OUTPUT);
      for (int i=0;i<3;i++) {
        digitalWrite(LED_BUILTIN,HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN,LOW);
        delay(500);
      }
    }

    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());
  
  //Push a message to LineNotify
  String message = encodeMessage("message=Test message\nI'm a \"Maker\"");
  String request = message;
  Serial.println(LineNotify(request, 1)); 

  //Push a emoji to LineNotify
  String message_emoji = encodeMessage("message=Test emoji\nI'm a \"Maker\"");    
  String emoji = "&stickerPackageId=1&stickerId=2";
  String request_emoji = message_emoji + emoji;
  Serial.println(LineNotify(request_emoji, 1)); 
  
  //Push a image to LineNotify
  String message_image = encodeMessage("message=Test image\nI'm a \"Maker\"");  
  String imageFullsize = "https://img.soundofhope.org/2020-09/lzl-1600614896059.jpg";
  String imageThumbnail = "https://img.ltn.com.tw/Upload/ent/page/800/2022/05/25/phpsBPOHo.jpg";
  String image = "&imageFullsize="+imageFullsize+"&imageThumbnail="+imageThumbnail;
  String request_image = message_image + image;
  Serial.println(LineNotify(request_image, 1)); 
}

void loop()
{
}

String LineNotify(String request, byte wait) { 
  TLSClient client_tcp;
  client_tcp.setRootCA(rootCA, sizeof(rootCA));
  
  if (client_tcp.connect("notify-api.line.me", 443)) {
    client_tcp.println("POST /api/notify HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: notify-api.line.me");
    client_tcp.println("User-Agent: ESP8266/1.0");
    client_tcp.println("Authorization: Bearer " + token);
    client_tcp.println("Content-Type: application/x-www-form-urlencoded");
    client_tcp.println("Content-Length: " + String(request.length()));
    client_tcp.println();
    client_tcp.println(request);
    client_tcp.println();
    
    String getResponse="",Feedback="";
    boolean state = false;
    int waitTime = 3000;   // timeout 3 seconds
    long startTime = millis();
    while ((startTime + waitTime) > millis()) {
      while (client_tcp.available()) {
          char c = client_tcp.read();
          if (state==true) Feedback += String(c);        
          if (c == '\n') {
            if (getResponse.length()==0) state=true; 
            getResponse = "";
          } 
          else if (c != '\r')
            getResponse += String(c);
          if (wait==1)
            startTime = millis();
       }
       if (wait==0)
        if ((state==true)&&(Feedback.length()!= 0)) break;
    }
    client_tcp.stop();
    return Feedback;
  }
  else
    return "Connection failed";  
}

String encodeMessage(String message) {
  message.replace("%","%25");  
  message.replace(" ","%20");
  message.replace("&","%20");
  message.replace("#","%20");
  //message.replace("\'","%27");
  message.replace("\"","%22");
  message.replace("\n","%0D%0A"); 
  return message;
}
