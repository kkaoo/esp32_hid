/*  Author: Leon 20190924
*/

#include "arduino.h"

#include "HIDTypes.h"

//You can use any pins on the board
//but I don't recommend the ones that are used

#define KEY_UP      27
#define KEY_DOWN    12
#define KEY_LEFT    14
#define KEY_RIGHT   13

#define KEY_A       16
#define KEY_B       18
#define KEY_X       17
#define KEY_Y       19   

#define KEY_L       25
#define KEY_LT      26

#define KEY_R       23
#define KEY_RT      22

#define KEY_TURBO   0
#define KEY_AUTO    KEY_TURBO
//#define KEY_MODE    KEY_TURBO

#define KEY_SELECT  5
#define KEY_START   KEY_SELECT

#define KEY_TURBO2  32
#define KEY_TURBO3  KEY_TURBO2

#define LEFT_VR1    39
#define LEFT_VR2    34

#define RIGHT_VR1   35
#define RIGHT_VR2   36

#define BATTERY_VOLT  33

#define LEFT_MOTOR  2
//#define RIGHT_MOTOR 4

#define VR_PWR      4

#define JOYSTICK_REPORT 0

#if JOYSTICK_REPORT==0
const uint8_t reportMapJoystick[] = { 
  USAGE_PAGE(1), 			        0x01,   // USAGE_PAGE (Generic Desktop)
  USAGE(1),                   0x05,   // USAGE (GAME PAD)
  COLLECTION(1),              0x01,   // COLLECTION (Application)
    REPORT_ID(1),             0x01,   // REPORT_ID (1)
    USAGE(1),                 0x01,   //(point)
    COLLECTION(1),            0x00,   // COLLECTION (Physical)
        USAGE(1), 				    0x30,   // X axis
        USAGE(1), 			      0x31,   // Y axis
        USAGE(1), 				    0x35,   // Z-rotator axis
        USAGE(1), 				    0x32,   // Z axis
        LOGICAL_MINIMUM(1),   0x00,   // LOGICAL_MINIMUM (0)
        LOGICAL_MAXIMUM(2),   0xff, 0x00,  // LOGICAL_MAXIMUM (255)
        REPORT_SIZE(1),       0x08,   // REPORT_SIZE (8)
        REPORT_COUNT(1),      0x04,   // REPORT_COUNT (1)
        HIDINPUT(1),          0x02,   // INPUT (Data,Var,Abs)
    END_COLLECTION(0),         // END_COLLECTION

    USAGE(1), 				    0x39,  //hat switch
    LOGICAL_MINIMUM(1),   0x00,
    LOGICAL_MAXIMUM(1),   0x07,
    PHYSICAL_MINIMUM(1),  0x00,
    PHYSICAL_MAXIMUM(2),  0x3B, 0x01,
    UNIT(1),              0x14,
    REPORT_SIZE(1),	  	  0x04,
    REPORT_COUNT(1),	    0x01,
    HIDINPUT(1),          0x42,  //variable | absolute | null state
    REPORT_SIZE(1),	  	  0x04,
    REPORT_COUNT(1),	    0x01,//0x01,
    HIDINPUT(1),          0x01,

    USAGE_PAGE(1),          0x09,   //(Button)
    USAGE_MINIMUM(1),     0x01,
    USAGE_MAXIMUM(1),     0x0C,   // 12
    LOGICAL_MINIMUM(1),   0x00,   // LOGICAL_MINIMUM (0)
    LOGICAL_MAXIMUM(1),   0x01,   // LOGICAL_MAXIMUM (1)
    REPORT_SIZE(1),	  	  0x01,   // REPORT_SIZE (1)
    REPORT_COUNT(1),      0x0C,   // REPORT_COUNT (16)
    UNIT_EXPONENT(1),     0x00,
    UNIT(1),              0x00,
    HIDINPUT(1),            0x02,   // variable | absolute

    REPORT_SIZE(1),	  	  0x01,   // REPORT_SIZE (1)
    REPORT_COUNT(1),      0x04,   // REPORT_COUNT (4)
    HIDINPUT(1),          0x01,   // variable | absolute

    // USAGE_PAGE(1),            0x8C,   //Bar Code Scanner
    // USAGE(1),                 0x01,   //(point)
    // COLLECTION(1),            0x00,   // COLLECTION (Physical)
    //   USAGE(1),               0x02,
    //     LOGICAL_MINIMUM(1),   0x00,   // LOGICAL_MINIMUM (0)
    //     LOGICAL_MAXIMUM(2),   0xFF,0x00,   // LOGICAL_MAXIMUM (1)

    //     REPORT_SIZE(1),	  	  0x08,   // REPORT_SIZE (1)
    //     REPORT_COUNT(1),      0x03,   // REPORT_COUNT (16)
    //     HIDOUTPUT(1),         0x02,
    //     USAGE(1),             0x02,
    //     FEATURE(1),           0x02,
        
    // END_COLLECTION(0),         // END_COLLECTION
  END_COLLECTION(0)         // END_COLLECTION
};
#elif JOYSTICK_REPORT==1
const uint8_t reportMapJoystick[] = { //This is where the amount, type, and value range of the inputs are declared
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x05, // USAGE (Gamepad)
    0xa1, 0x01, // COLLECTION (Application)
    0x85, 0x01, //   REPORT_ID (1)

      0x15, 0x00, // LOGICAL_MINIMUM (0)
      0x25, 0x01, // LOGICAL_MAXIMUM (1)
      0x35, 0x00, // PHYSICAL_MINIMUM (0)
      0x45, 0x01, // PHYSICAL_MAXIMUM (1)
      0x75, 0x01, // REPORT_SIZE (1)
      0x95, 0x10, // REPORT_COUNT (16)
      0x05, 0x09, // USAGE_PAGE (Button)
      0x19, 0x01, // USAGE_MINIMUM (Button 1)
      0x29, 0x10, // USAGE_MAXIMUM (Button 16)
      0x81, 0x02, // INPUT (Data,Var,Abs)

      0x05, 0x01, // USAGE_PAGE (Generic Desktop)
      0x26, 0xff, 0x00, // LOGICAL_MAXIMUM (255)
      0x46, 0xff, 0x00, // PHYSICAL_MAXIMUM (255)
      USAGE(1),   0x30, // USAGE (X)
      USAGE(1),   0x31,
      USAGE(1),   0x32,
      USAGE(1),   0x35,
      0x75, 0x08, // REPORT_SIZE (8)
      0x95, 0x04, // REPORT_COUNT (1)
      0x81, 0x02, // INPUT (Data,Var,Abs)

      USAGE(1), 				    0x39,  //hat switch
      LOGICAL_MINIMUM(1),   0x00,
      LOGICAL_MAXIMUM(1),   0x07,
      PHYSICAL_MINIMUM(1),  0x00,
      PHYSICAL_MAXIMUM(2),  0x3B, 0x01,
      UNIT(1),              0x14,
      REPORT_SIZE(1),	  	  0x04,
      REPORT_COUNT(1),	    0x01,
      HIDINPUT(1),          0x42,  //variable | absolute | null state
      REPORT_SIZE(1),	  	  0x04,
      REPORT_COUNT(1),	    0x01,//0x01,
      HIDINPUT(1),          0x01,      

    0xc0 // END_COLLECTION
};

#else

  const uint8_t reportMapJoystick[] = { //This is where the amount, type, and value range of the inputs are declared
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x05, // USAGE (Gamepad)
    0xa1, 0x01, // COLLECTION (Application)
    0x85, 0x01, //   REPORT_ID (1)

      0x15, 0x00, // LOGICAL_MINIMUM (0)
      0x25, 0x01, // LOGICAL_MAXIMUM (1)
      0x35, 0x00, // PHYSICAL_MINIMUM (0)
      0x45, 0x01, // PHYSICAL_MAXIMUM (1)
      0x75, 0x01, // REPORT_SIZE (1)
      0x95, 0x10, // REPORT_COUNT (16)
      0x05, 0x09, // USAGE_PAGE (Button)
      0x19, 0x01, // USAGE_MINIMUM (Button 1)
      0x29, 0x10, // USAGE_MAXIMUM (Button 16)
      0x81, 0x02, // INPUT (Data,Var,Abs)

      0x05, 0x01, // USAGE_PAGE (Generic Desktop)
      0x26, 0xff, 0x00, // LOGICAL_MAXIMUM (255)
      0x46, 0xff, 0x00, // PHYSICAL_MAXIMUM (255)
      0x09, 0x31, // USAGE (Y)
      0x75, 0x08, // REPORT_SIZE (8)
      0x95, 0x01, // REPORT_COUNT (1)
      0x81, 0x02, // INPUT (Data,Var,Abs)

    0xc0 // END_COLLECTION
  };
#endif

/** @brief One command (report) to be issued via BLE joystick profile
 * @see joystick_q */
typedef struct joystick_command {
  /** @brief Button mask, allows up to 32 different buttons */
  uint32_t buttonmask;
  /** @brief X-axis value, 0-1023 */
  uint16_t Xaxis;
  /** @brief Y-axis value, 0-1023 */
  uint16_t Yaxis;
  /** @brief Z-axis value, 0-1023 */
  uint16_t Zaxis;
  /** @brief Z-rotate value, 0-1023 */
  uint16_t Zrotate;
  /** @brief Slider left value, 0-1023 */
  uint16_t sliderLeft;
  /** @brief Slider right value, 0-1023 */
  uint16_t sliderRight;
  /** @brief Hat position (0-360), mapped to 8 directions. Use <0 for no pressing*/
  int16_t hat;

  uint8_t vr_left;
  uint8_t vr_right;
} joystick_command_t;