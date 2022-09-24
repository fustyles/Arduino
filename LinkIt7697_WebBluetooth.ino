/*
  This example configures LinkIt 7697 to act as a simple GATT server with 1 characteristic.

  To use it, open AppInventor project:

    * 

  Build & install it on Android id

  created Mar 2017
*/
#include <LBLE.h>
#include <LBLEPeriphral.h>

// Define a simple GATT service with only 1 characteristic
LBLEService SERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
LBLECharacteristicInt CHARACTERISTIC_UUID_RX("beb5483e-36e1-4688-b7f5-ea07361b26a8", LBLE_READ | LBLE_WRITE);
LBLECharacteristicInt CHARACTERISTIC_UUID_TX("498c599b-2601-4600-bb7e-3aa295a92842", LBLE_READ | LBLE_WRITE);

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  // to check if USR button is pressed
  pinMode(6, INPUT);  
  
  // Initialize BLE subsystem
  LBLE.begin();
  while (!LBLE.ready()) {
    delay(100);
  }
  Serial.println("BLE ready");

  Serial.print("Device Address = [");
  Serial.print(LBLE.getDeviceAddress());
  Serial.println("]");

  // configure our advertisement data.
  // In this case, we simply create an advertisement that represents an
  // connectable device with a device name
  LBLEAdvertisementData advertisement;
  advertisement.configAsConnectableDevice("BLE LinkIt7697");

  // Configure our device's Generic Access Profile's device name
  // Ususally this is the same as the name in the advertisement data.
  LBLEPeripheral.setName("BLE LinkIt7697");

  // Add characteristics into SERVICE_UUID
  SERVICE_UUID.addAttribute(CHARACTERISTIC_UUID_RX);
  SERVICE_UUID.addAttribute(CHARACTERISTIC_UUID_TX);
  
  // Add service to GATT server (peripheral)
  LBLEPeripheral.addService(SERVICE_UUID);

  // start the GATT server - it is now 
  // available to connect
  LBLEPeripheral.begin();

  // start advertisment
  LBLEPeripheral.advertise(advertisement);
}

void loop() {
  
  if (digitalRead(6)) {
    Serial.println("disconnect all!");
    LBLEPeripheral.disconnectAll();
  }

  if (LBLEPeripheral.connected()==1) {
    if (CHARACTERISTIC_UUID_RX.isWritten()) {
      const char value = CHARACTERISTIC_UUID_RX.getValue();
      Serial.println(value);
      CHARACTERISTIC_UUID_TX.setValue(value);
  
      // broadcasting value changes to all connected central devices
      LBLEPeripheral.notifyAll(CHARACTERISTIC_UUID_TX);
    }
  }
  
  delay(100);
}
