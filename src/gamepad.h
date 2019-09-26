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

#define KEY_TURBO   33
#define KEY_AUTO    KEY_TURBO

#define KEY_SELECT  5
#define KEY_START   KEY_SELECT
#define KEY_MODE    0

#define KEY_TURBO2  32
#define KEY_TURBO3  KEY_TURBO2

#define LEFT_VR1    39
#define LEFT_VR2    34

#define RIGHT_VR1   35
#define RIGHT_VR2   36

#define LEFT_MOTOR  2
#define RIGHT_MOTOR 4



#define INPUT_(size)             (0x80 | size)

const uint8_t reportMapJoystick[] = { //This is where the amount, type, and value range of the inputs are declared
  USAGE_PAGE(1), 			      0x01,   // USAGE_PAGE (Generic Desktop)
  USAGE(1),                 0x05,   // USAGE (Gamepad)
  COLLECTION(1),            0x01,   // COLLECTION (Application)
  REPORT_ID(1),             0x01,   // REPORT_ID (1)
  
  LOGICAL_MINIMUM(1),       0x00,   // LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1),       0x01,   // LOGICAL_MAXIMUM (1)
  PHYSICAL_MINIMUM(1),      0x00,   // PHYSICAL_MINIMUM (0)
  PHYSICAL_MAXIMUM(1),      0x01,   // PHYSICAL_MAXIMUM (1)
  REPORT_SIZE(1),	  	      0x01,   // REPORT_SIZE (1)
  REPORT_COUNT(1),          0x10,   // REPORT_COUNT (16)
  USAGE_PAGE(1),            0x09,   //(Button)
      USAGE_MINIMUM(1),     0x01,
      USAGE_MAXIMUM(1),     0x10,   // 16
      INPUT_(1),            0x02,   // variable | absolute

  LOGICAL_MAXIMUM(2),   0xff, 0x00, // LOGICAL_MAXIMUM (255)
  PHYSICAL_MAXIMUM(2),  0xff, 0x00, // PHYSICAL_MAXIMUM (255)
  REPORT_SIZE(1),           0x08,   // REPORT_SIZE (8)
  REPORT_COUNT(1),          0x04,   // REPORT_COUNT (1)
  USAGE_PAGE(1),            0x01,   // USAGE_PAGE (Generic Desktop)
      USAGE(1), 				    0x30,   // X axis
      USAGE(1), 			      0x31,   // Y axis
      USAGE(1), 				    0x32,   // Z axis
      USAGE(1), 				    0x35,   // Z-rotator axis
      INPUT_(1),            0x02,   // INPUT (Data,Var,Abs)

  LOGICAL_MINIMUM(1),       0x00,
  LOGICAL_MAXIMUM(1),       0x07,
  PHYSICAL_MINIMUM(1),      0x01,
  PHYSICAL_MAXIMUM(2),      (315 & 0xFF), ((315>>8) & 0xFF),
  REPORT_SIZE(1),	  	      0x04,
  REPORT_COUNT(1),	        0x01,
  UNIT(1),                  20,
  USAGE_PAGE(1), 			      0x01,
      USAGE(1), 				    0x39,  //hat switch
      INPUT_(1),            0x42,  //variable | absolute | null state

  END_COLLECTION(0)         // END_COLLECTION
};


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
} joystick_command_t;