/*
Web Bluetooth for ESP32
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-1-15 15:30
https://www.facebook.com/francefu

Try it
https://fustyles.github.io/webduino/WebBluetooth.html
*/


//https://www.instructables.com/ESP32-Bluetooth-Low-Energy/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *characteristicTX;
bool deviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_RX "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_TX "498c599b-2601-4600-bb7e-3aa295a92842"

class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected!");
    }
};

//callback  para envendos das características
class CharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *characteristic) {
      std::string rxValue = characteristic->getValue(); 
      if (rxValue.length() > 0) {
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }

        Serial.println();

        if (rxValue.find("on") != -1) { 
          Serial.println("Turning LED ON!");
          characteristicTX->setValue("Turning LED ON!");
          characteristicTX->notify();          
        }
        else  if (rxValue.find("off") != -1) { 
          Serial.println("Turning LED OFF!");
          characteristicTX->setValue("Turning LED OFF!");
          characteristicTX->notify();          
        }

        /*
        float txValue = 100; // This could be an actual sensor reading!
        char txString[8]; // make sure this is big enuffz
        dtostrf(txValue, 1, 2, txString); // float_val, min_width, digits_after_decimal, char_buffer
        //characteristicTX->setValue(txValue, 1); // To send the integer value
        //characteristicTX->setValue("Hello!"); // Sending a test message
        characteristicTX->setValue(txString);
        characteristicTX->notify();
        */
      }
    }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("ESP32-BLE");
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());
  
  BLEService *service = server->createService(SERVICE_UUID);
  
  characteristicTX = service->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE |
                      BLECharacteristic::PROPERTY_BROADCAST 
                    );
                      
  characteristicTX->addDescriptor(new BLE2902());

  BLECharacteristic *characteristic = service->createCharacteristic(
                                        CHARACTERISTIC_UUID_RX,
                                        BLECharacteristic::PROPERTY_READ   |
                                        BLECharacteristic::PROPERTY_WRITE  |
                                        BLECharacteristic::PROPERTY_NOTIFY |
                                        BLECharacteristic::PROPERTY_INDICATE |
                                        BLECharacteristic::PROPERTY_BROADCAST                              
                                       );

  characteristic->setCallbacks(new CharacteristicCallbacks());
  service->start();
  server->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
}
