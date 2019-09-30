/*  Author: Leon
*/

#include <BLEDevice.h>  //Libraries necessary for program
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include <driver/adc.h>
#include <driver/ledc.h>

#include "esp_pm.h"
#include "esp_bt_main.h"
#include "esp_wifi.h"

#include "arduino.h"
#include "gamepad.h"

#define LOG_TAG "ESP_HID"
#define GAMEPAD_TICK_MS             2
#define SINGLE_KEY_DEBOUNCES_MS     10
#define SCAN_KEY_DEBOUNCES_MS       32


BLEHIDDevice* hid;          //declare hid device
BLECharacteristic* input;   //Characteristic that inputs button values to devices
BLECharacteristic* output;  //Characteristic that takes input from client

//Stores if the board is connected or not
bool connected = false;
bool mode = false;

//pin that goes high while there's a device connected
#define CONNECTED_LED_INDICATOR_PIN 15

ledc_channel_config_t* led_channel_1 = 0;
float led_duty_1 = 0;

void led_set_duty(float duty){
  led_duty_1 = duty;
};

void led_update_duty(float duty){
  if(duty > 100) duty = 100;

  if(led_channel_1)
  if(led_duty_1!=duty)
  {
    led_set_duty(duty);
    ledc_set_duty(led_channel_1->speed_mode, led_channel_1->channel, uint32_t(5000/100 * led_duty_1));
    ledc_update_duty(led_channel_1->speed_mode, led_channel_1->channel);
  }
};


/**
 * @brief 初始化指示LED
 */
void led_init(void){
  ledc_timer_config_t ledc_timer = {
          (ledc_mode_t)LEDC_HIGH_SPEED_MODE,           // timer mode
          (ledc_timer_bit_t)LEDC_TIMER_13_BIT,
          (ledc_timer_t)LEDC_TIMER_0,
          (ledc_timer_t)5000
  };
  ledc_timer_config(&ledc_timer);

  static ledc_channel_config_t ledc_channel = {
          CONNECTED_LED_INDICATOR_PIN,
          (ledc_mode_t)LEDC_HIGH_SPEED_MODE,
          (ledc_channel_t)LEDC_CHANNEL_0,
          (ledc_intr_type_t)LEDC_INTR_DISABLE,
          (ledc_timer_t)LEDC_TIMER_0,
          0,
          0
  };
  ledc_channel_config(&ledc_channel);
  // ledc_fade_func_install(0);

  led_channel_1 = &ledc_channel;

  led_update_duty(5);
  delay(200);
  led_update_duty(0);
}

int sleep_count = 0;
void sleep_clear(void){ sleep_count = 0; }

class MyCallbacks : public BLEServerCallbacks { //Class that does stuff when device disconects or connects
    void onConnect(BLEServer* pServer) {
      // connected = true;
      Serial.println("Connected ");
      // Serial.println((rtc_clk_cpu_freq_get() * 80));
      BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
      desc->setNotifications(true);

      // digitalWrite(CONNECTED_LED_INDICATOR_PIN, HIGH);
      // led_set_duty(5);
    }

    void onDisconnect(BLEServer* pServer) {
      connected = false;
      Serial.println("Disconnected");
      BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
      desc->setNotifications(false);
      
      // digitalWrite(CONNECTED_LED_INDICATOR_PIN, LOW);
      // led_set_duty(0);
    }
};

class MyOutputCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* me) {
      uint8_t* value = (uint8_t*)(me->getValue().c_str());
      ESP_LOGD(LOG_TAG, "special keys: %d", *value);
      // Serial.printf("special keys: %s", *value);
    }
};

uint32_t passKey = 1307;
/** @brief security callback
 * 
 * This class is passed to the BLEServer as callbacks for security
 * related actions. Depending on IO_CAP configuration & host, different
 * types of security actions are required for bonding this device to a
 * host. */
class CB_Security: public BLESecurityCallbacks {
  
  // Request a pass key to be typed in on the host
  uint32_t onPassKeyRequest(){
    ESP_LOGE(LOG_TAG, "The passkey request %d", passKey);
    vTaskDelay(25000);
    return passKey;
  }
  
  // The host sends a pass key to the ESP32 which needs to be displayed
  //and typed into the host PC
  void onPassKeyNotify(uint32_t pass_key){
    ESP_LOGE(LOG_TAG, "The passkey Notify number:%d", pass_key);
    passKey = pass_key;
  }
  
  // CB for testing if a host is allowed to connect, in our case always yes.
  bool onSecurityRequest(){
    return true;
  }

  // CB on a completed authentication (not depending on status)
  void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl){
    if(auth_cmpl.success){
      ESP_LOGI(LOG_TAG, "remote BD_ADDR:");
      esp_log_buffer_hex(LOG_TAG, auth_cmpl.bd_addr, sizeof(auth_cmpl.bd_addr));
      ESP_LOGI(LOG_TAG, "address type = %d", auth_cmpl.addr_type);

      connected = true;
    }
      ESP_LOGI(LOG_TAG, "pair status = %s", auth_cmpl.success ? "success" : "fail");
  }
  
  // You need to confirm the given pin
  bool onConfirmPIN(uint32_t pin)
  {
    ESP_LOGE(LOG_TAG, "Confirm pin: %d", pin);
    return true;
  }
  
};

void taskServer(void*) {

  BLEDevice::init("GAME-CONTROLLER");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyCallbacks());
	// BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_NO_MITM);
  BLEDevice::setSecurityCallbacks(new CB_Security());

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
  hid->setBatteryLevel(50);

  ESP_LOGD(LOG_TAG, "Advertising started!");

  delay(portMAX_DELAY);

};

/**
 * @brief 单键扫描
 */
class SingleKey{
  public:
    int deb_count;        //防抖计数器
    int DEBOUNCES_TIME;   //deb_count和DEBOUNCES_TIME相等时，防抖通过
    int status;           //Key最新的状态值
    int pin_num;          //
    bool invert;          //是否反转电平
    bool trigger;         //按键被触发了

    SingleKey(uint8_t _pin_num, bool _invert=false, int _debounces_time=5){
      deb_count = 0;
      status = 0;
      pin_num = _pin_num;
      invert = _invert;
      DEBOUNCES_TIME = _debounces_time;
      trigger = false;
    }

    void update(void){
      int newkey = digitalRead(pin_num);
      if(invert){
        newkey = newkey==1?0:1;
      }
      if(newkey != status){ 
        deb_count++;
        if(deb_count >= DEBOUNCES_TIME){
          deb_count = 0;
          status = newkey;
          if(status)
            trigger = true;
        }
      }else{
        deb_count = 0;
      }
    }
};

/* enum定义 和 const定义 排序要对称 */
enum{               INX_UP=0, INX_RIGHT, INX_DOWN, INX_LEFT, INX_Y, INX_B, INX_A, INX_X, INX_L, INX_R, INX_LT, INX_RT, INX_SELECT, INX_START, INX_TURBO3, INX_TURBO2, INX_MODE, INX_TURBO, INX_AUTO };
const uint8_t KEYS[]={KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_LEFT, KEY_Y, KEY_B, KEY_A, KEY_X, KEY_L, KEY_R, KEY_LT, KEY_RT, KEY_SELECT, KEY_START, KEY_TURBO3, KEY_TURBO2, KEY_MODE, KEY_TURBO, KEY_AUTO };
static joystick_command_t jcmd;

// uint16_t value_limit(uint16_t value){
//   if(value >= 128+32)
//     return 255;
//   else if(value <= 128-32)
//     return 0;
//   else
//     return 128;
// }

/**
 * @brief GamePAD任务, 处理按键动作, 上传数据
 **/
void joystickTask(void*) {
  SingleKey* m_keys[sizeof(KEYS)];
  for(int i=0; i<sizeof(KEYS); i++){
    pinMode(KEYS[i], INPUT_PULLUP);

    bool invert = ((i==INX_SELECT || i==INX_TURBO || i==INX_TURBO2))?false:true;
    int DEBS = SINGLE_KEY_DEBOUNCES_MS/GAMEPAD_TICK_MS;
    if(KEYS[i]==KEY_SELECT || KEYS[i]==KEY_TURBO || KEYS[i]==KEY_TURBO2)
      DEBS = (SCAN_KEY_DEBOUNCES_MS/2/GAMEPAD_TICK_MS);
    m_keys[i] = new SingleKey(KEYS[i],invert,DEBS);
  }
  // Serial.println(m_keys[INX_SELECT]->DEBOUNCES_TIME);
  // Serial.println(m_keys[INX_A]->DEBOUNCES_TIME);

  // uint8_t cycle = 0;
  uint8_t current_mode = INPUT_PULLUP;
  while(true){
    delay(GAMEPAD_TICK_MS);

    // 扫描所有按键
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

    // 扫描VR并简易滤波
    static uint32_t total[4];
    static uint16_t VR[4];
    static int total_cycle = 0;
    static uint8_t vr_left_status = 0;
    static uint8_t vr_right_status = 0;
    total[0]+=(4095-analogRead(LEFT_VR2));
    total[1]+=(4095-analogRead(LEFT_VR1));
    total[2]+=(4095-analogRead(RIGHT_VR2));
    total[3]+=(4095-analogRead(RIGHT_VR1));
    if((++total_cycle) >= 4){
      for(int i=0; i<4; i++){
        VR[i] = total[i]/total_cycle/16;
        total[i] = 0;
      }
      total_cycle = 0;
      vr_left_status = vr_right_status = 0;
      if(VR[1]<=96) vr_left_status|=0x01;
      if(VR[1]>=160) vr_left_status|=0x04;
      if(VR[0]<=96) vr_left_status|=0x08;
      if(VR[0]>=160) vr_left_status|=0x02;

      if(VR[3]<=96) vr_right_status|=0x01;
      if(VR[3]>=160) vr_right_status|=0x04;
      if(VR[2]<=96) vr_right_status|=0x08;
      if(VR[2]>=160) vr_right_status|=0x02;
    }    

      //模式按钮，进行模式切换
      if(m_keys[INX_MODE]->trigger){
        m_keys[INX_MODE]->trigger = 0;
        mode = !mode;
      }

      //按键分析 
      uint32_t mask=0;      //记录bit单位的按键状态
      int16_t hat = 0;      //记录方向盘状态

      for(int i=0; i<4; i++)
        hat|=(m_keys[i]->status<<i);
      for(int i=4; i<INX_MODE; i++)
        mask|=(m_keys[i]->status<<(i-4));
      const uint8_t HAT[] = {0,1,3,2, 5,0,4,0, 7,8,0,0, 6,0,0,0};

      if(mode){
        if(hat==0) hat=vr_left_status;          //左摇杆映射到UDLR方向盘
        mask = mask | (vr_right_status&0x0f);   //右摇杆映射到ABXO
        jcmd.Xaxis=jcmd.Yaxis=jcmd.Zaxis=jcmd.Zrotate=0x80;
      }else{
        jcmd.Xaxis = VR[0]; jcmd.Yaxis = VR[1]; jcmd.Zaxis = VR[2]; jcmd.Zrotate = VR[3];
      }
      hat = HAT[hat];

      //如果连接状态，那么可以发送数据，发送有二种机制
      //1 如果有按键按下立即发送
      //2 如果摇感有摇动，发送持续发送10S, 间隔50MS 
      //    持续发送在映射模式不起作用，因为映射模式没有摇杆数据，不需要进行持续更新
      static int cmd_send_count_down = 0;
      static int cmd_send_delay = 0;
      if (connected){
        if( jcmd.vr_left!=vr_left_status || jcmd.vr_right!=vr_right_status ){
          cmd_send_delay = 0;
          if(mode!=true)
            cmd_send_count_down = 10 * 1000 / GAMEPAD_TICK_MS; 
        }else if( jcmd.buttonmask!=mask || jcmd.hat!=hat ){
          cmd_send_delay = 0;
        }else if(cmd_send_count_down){
          if(cmd_send_delay) cmd_send_delay--;
        }
        else
          cmd_send_delay = 100;
        
        if(cmd_send_count_down) cmd_send_count_down--;

        if(cmd_send_delay==0){
          cmd_send_delay = 50 / GAMEPAD_TICK_MS;
          jcmd.buttonmask = mask;
          jcmd.hat = hat;
          jcmd.vr_left = vr_left_status;
          jcmd.vr_right = vr_right_status;

          uint8_t a[] = {0,0,0,0,0,0,0};
          a[0] = jcmd.buttonmask & 0xff;
          a[1] = (jcmd.buttonmask>>8) & 0xff;
          
          a[2] = (jcmd.Xaxis) & 0xff;
          a[3] = (jcmd.Yaxis) & 0xff;
          a[4] = (jcmd.Zaxis) & 0xff;
          a[5] = (jcmd.Zrotate) & 0xff;

          a[6] = jcmd.hat & 0xff;

          input->setValue(a, sizeof(a));
          input->notify();
          // Serial.printf("%2x  %2x  %2x  %2x  %2x  %2x  %2x  %2x  %2x\n",a[0],a[1],a[2],a[3],a[4],a[5],a[6],vr_left_status,vr_right_status);
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


RTC_DATA_ATTR int bootPasscode1 = 0; //将变量存放于RTC Memory
RTC_DATA_ATTR int bootPasscode2 = 0; //将变量存放于RTC Memory
RTC_DATA_ATTR int bootCount = 0; //将变量存放于RTC Memory

void setup() {
  Serial.begin(115200);

  bootCount++; //累加计数值
  Serial.printf("这是第 %d 次复位n", bootCount);
  switch(esp_sleep_get_wakeup_cause()) //获取唤醒原因
  {
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("通过定时器唤醒"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("通过触摸唤醒"); break;
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("通过EXT0唤醒"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("通过EXT1唤醒"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("通过ULP唤醒"); break;
    default : Serial.println("并非从DeepSleep中唤醒"); break;
  }

  pinMode(LEFT_MOTOR, OUTPUT);
  pinMode(RIGHT_MOTOR, OUTPUT);
  digitalWrite(RIGHT_MOTOR, HIGH);

  led_init();

  // xTaskCreate(my_test, "my_test", 2048, NULL, 1, NULL);
  xTaskCreate(joystickTask, "joystick", 20000, NULL, 1, NULL);
  xTaskCreate(taskServer, "server", 20000, NULL, 5, NULL);

  sleep_clear();
}


unsigned int flash_cycle;
void loop() {
  if(connected){
    led_update_duty(int((3000-sleep_count)/30)/50 * mode?1:3);
  }
  else{
    flash_cycle++;
    if(flash_cycle & 0x0000004)
      led_update_duty(2);
    else
      led_update_duty(0);
  }

  delay(100);
  if(connected){
        // 检测手柄是否有按键动作
        static joystick_command_t cmdst;
        if(jcmd.buttonmask!=cmdst.buttonmask || jcmd.hat!= cmdst.hat ||
          (jcmd.Xaxis&0xc0) != (cmdst.Xaxis&0xc0) || (jcmd.Zaxis&0xc0) != (cmdst.Zaxis&0xc0) ){
          cmdst = jcmd;
          sleep_clear();
        }
  }

  sleep_count ++;
  if(sleep_count >= (300*1000/100)){
    sleep_count = 0;
    Serial.printf("Enabling EXT0 wakeup on pins GPIO%d\n", KEY_LEFT);
    Serial.flush();

    delay(50);
    esp_sleep_enable_ext1_wakeup(uint64_t((uint64_t)1<<KEY_LEFT), ESP_EXT1_WAKEUP_ALL_LOW);
    delay(50);
    esp_deep_sleep_start(); //启动DeepSleep
  }

  // Serial.println(sleep_count);
  // Serial.println(jcmd.Xaxis);
}
