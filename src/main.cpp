/*  Author: Leon, 20190925
*/

#include <BLEDevice.h>  //Libraries necessary for program
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include <driver/adc.h>

#include "arduino.h"
#include "gamepad.h"

BLEHIDDevice* hid;          //declare hid device
BLECharacteristic* input;   //Characteristic that inputs button values to devices
BLECharacteristic* output;  //Characteristic that takes input from client

//Stores if the board is connected or not
bool connected = false;

//pin that goes high while there's a device connected
#define CONNECTED_LED_INDICATOR_PIN 15

//inputValues[0] is the first 8 buttons, [1] is the next 8, [2] is the analog input
//Each one of the bits represnets a button. 1 == pressed 0 == not pressed
uint8_t inputValues[3] = {0b00000000, 0b00000000, 0x0};

class MyCallbacks : public BLEServerCallbacks { //Class that does stuff when device disconects or connects
    void onConnect(BLEServer* pServer) {
      connected = true;
      Serial.println("Connected");
      BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
      desc->setNotifications(true);

      digitalWrite(CONNECTED_LED_INDICATOR_PIN, HIGH);
    }

    void onDisconnect(BLEServer* pServer) {
      connected = false;
      Serial.println("Disconnected");
      BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
      desc->setNotifications(false);
      
      digitalWrite(CONNECTED_LED_INDICATOR_PIN, LOW);
    }
};

class MyOutputCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* me) {
      uint8_t* value = (uint8_t*)(me->getValue().c_str());
      //ESP_LOGI(LOG_TAG, "special keys: %d", *value);
    }
};


void taskServer(void*) {

  BLEDevice::init("GAME-CONTROLLER");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyCallbacks());

  hid = new BLEHIDDevice(pServer);
  input = hid->inputReport(1); // <-- input REPORTID from report map
  output = hid->outputReport(1); // <-- output REPORTID from report map

  output->setCallbacks(new MyOutputCallbacks());

  std::string name = "LEON";
  hid->manufacturer()->setValue(name);

  hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  hid->hidInfo(0x00, 0x02);

  BLESecurity *pSecurity = new BLESecurity();
  //  pSecurity->setKeySize();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

  hid->reportMap((uint8_t*)reportMapJoystick, sizeof(reportMapJoystick));
  // hid->reportMap((uint8_t*)reportMapJoystick, sizeof(reportMapJoystick));
  hid->startServices();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_GAMEPAD);
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->start();
  hid->setBatteryLevel(7);

  ESP_LOGD(LOG_TAG, "Advertising started!");
  delay(portMAX_DELAY);

};


class SimpleKey{
  public:
    int deb;
    int status;
    int key_num;
    bool invert;

    SimpleKey(uint8_t _key_num, bool _invert=false){
      deb = 0;
      status = 0;
      key_num = _key_num;
      invert = _invert;
    }

    void update(void){
      int newkey = digitalRead(key_num);
      if(invert){
        newkey = newkey==1?0:1;
      }
      if(newkey != status){ 
        deb++;
        if(deb >= 5){
          deb = 0;
          status = newkey;
        }
      }else{
        deb = 0;
      }
    }
};

enum{               INX_UP=0, INX_DOWN, INX_LEFT, INX_RIGHT, INX_Y, INX_B, INX_A, INX_X, INX_L, INX_R, INX_LT, INX_RT, INX_SELECT, INX_START, INX_TURBO3, INX_TURBO2, INX_MODE, INX_TURBO, INX_AUTO };
const uint8_t KEYS[]={KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_Y, KEY_B, KEY_A, KEY_X, KEY_L, KEY_R, KEY_LT, KEY_RT, KEY_SELECT, KEY_START, KEY_TURBO3, KEY_TURBO2, KEY_MODE, KEY_TURBO, KEY_AUTO };

joystick_command_t jcmd;

void joystickTask(void*) {
  SimpleKey* m_keys[sizeof(KEYS)];
  for(int i=0; i<sizeof(KEYS); i++){
    pinMode(KEYS[i], INPUT_PULLUP);

    bool invert = ((i==INX_SELECT || i==INX_TURBO || i==INX_TURBO2))?false:true;
    m_keys[i] = new SimpleKey(KEYS[i],invert);
  }

  uint8_t cycle = 0;
  uint8_t current_mode = INPUT_PULLUP;
  while(1){
    delay(2);
    for(int i=0; i<sizeof(KEYS); i++){
      if(current_mode == INPUT_PULLDOWN){
        if(!(i==INX_START || i==INX_AUTO || i==INX_TURBO3))
          m_keys[i]->update();
      }else{
        if(!(i==INX_SELECT || i==INX_TURBO || i==INX_TURBO2))
          m_keys[i]->update();
      }
    }

    if(current_mode == INPUT_PULLUP){
      current_mode = INPUT_PULLDOWN;
    }else{
      current_mode = INPUT_PULLUP;
    }
    pinMode(KEY_START, current_mode);
    pinMode(KEY_AUTO, current_mode);
    pinMode(KEY_TURBO3, current_mode);

    uint32_t mask=0;
    int16_t hat = 8;
    if(m_keys[INX_UP]->status){
      hat = 0;
      if(m_keys[INX_RIGHT]->status)
        hat = 1;
      else if(m_keys[INX_LEFT]->status)
        hat = 7;
    }
    else if(m_keys[INX_DOWN]->status){
      hat = 4;
      if(m_keys[INX_RIGHT]->status)
        hat = 3;
      else if(m_keys[INX_LEFT]->status)
        hat = 5;
    }
    else if(m_keys[INX_LEFT]->status)
      hat = 6;
    else if(m_keys[INX_RIGHT]->status)
      hat = 2;


    for(int i=4; i<sizeof(KEYS); i++)
      mask|=(m_keys[i]->status<<(i-4));

    static uint32_t total[4];

    total[0]+=(4095-analogRead(LEFT_VR2));
    total[1]+=(4095-analogRead(LEFT_VR1));
    total[2]+=(4095-analogRead(RIGHT_VR2));
    total[3]+=(4095-analogRead(RIGHT_VR1));

    cycle++;
    if(cycle >= 5){
      jcmd.Xaxis = total[0]/cycle/16 & 0xffffffff;
      jcmd.Yaxis = total[1]/cycle/16 & 0xffffffff;
      jcmd.Zaxis = total[2]/cycle/16 & 0xffffffff;
      jcmd.Zrotate = total[3]/cycle/16 & 0xffffffff;

      jcmd.hat = hat;
      jcmd.buttonmask = mask;

      // debug output
      // Serial.print(hat);
      // Serial.print(' ');
      // Serial.print(mask);
      // Serial.print(' ');
      // for(int i=0; i<sizeof(KEYS); i++)
      //   Serial.print(m_keys[i]->status);
      // Serial.print(' ');
      // Serial.print(jcmd.Xaxis);
      // Serial.print(' ');
      // Serial.print(jcmd.Yaxis);
      // Serial.print(' ');
      // Serial.print(jcmd.Zaxis);
      // Serial.print(' ');
      // Serial.print(jcmd.Zrotate);
      // Serial.println(' ');

      total[0] = total[1] = total[2] = total[3] = 0;
      cycle = 0;

      if (connected){
        static uint8_t a[] = {0,0,0,0,0,0,0};
        int i=0;
        a[i++] = jcmd.buttonmask & 0xff;
        a[i++] = (jcmd.buttonmask>>8) & 0xff;
        
        a[i++] = (jcmd.Xaxis) & 0xff;
        a[i++] = (jcmd.Yaxis) & 0xff;
        a[i++] = (jcmd.Zaxis) & 0xff;
        a[i++] = (jcmd.Zrotate) & 0xff;

        a[i++] = jcmd.hat & 0xff;
        input->setValue(a, sizeof(a));
        input->notify();
      }
    }
  }
}

void my_test(void*) {
  for(int i=0; i<sizeof(KEYS); i++)
    pinMode(KEYS[i], INPUT_PULLUP);

  digitalWrite(LEFT_MOTOR, HIGH);
  delay(2000);
  digitalWrite(LEFT_MOTOR, LOW);
  delay(1000);

  digitalWrite(RIGHT_MOTOR, HIGH);
  delay(2000);
  digitalWrite(RIGHT_MOTOR, LOW);
  delay(1000);

  while(1)
  {
    for(int i=0; i<sizeof(KEYS); i++)
      Serial.print(digitalRead(KEYS[i]));

    Serial.print(' ');
    Serial.print(analogRead(LEFT_VR1));

    Serial.print(' ');
    Serial.print(analogRead(LEFT_VR2));

    Serial.print(' ');
    Serial.print(analogRead(RIGHT_VR1));

    Serial.print(' ');
    Serial.print(analogRead(RIGHT_VR2));

    Serial.println(' ');
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LEFT_MOTOR, OUTPUT);
  pinMode(RIGHT_MOTOR, OUTPUT);
  pinMode(CONNECTED_LED_INDICATOR_PIN, OUTPUT);

  digitalWrite(CONNECTED_LED_INDICATOR_PIN, HIGH);
  delay(200);
  digitalWrite(CONNECTED_LED_INDICATOR_PIN, LOW);

  // xTaskCreate(my_test, "my_test", 2048, NULL, 1, NULL);
  xTaskCreate(joystickTask, "joystick", 20000, NULL, 1, NULL);
  xTaskCreate(taskServer, "server", 20000, NULL, 5, NULL);
}


void loop() {
}
