#include "CST820.h"
// #include <bitset>


/*
    some parts barrowed from fbiego's CST816S driver
    fbiego/CST816S
    https://pan.jczn1688.com/pd/1/HMI%20display/JC2432W328.zip
*/

CST820::CST820(void) {
}

void CST820::begin(int8_t _sda, int8_t _scl, int8_t _rst, int8_t _int) {
    int8_t x;
    int i;

    // Initialize I2C
    if (_sda != -1 && _scl != -1) {
        Wire.begin(_sda, _scl);
    } else {
        Wire.begin();
    }

    // Int Pin Configuration
    if (_int != -1) {
        pinMode(_int, OUTPUT);
        digitalWrite(_int, HIGH);
        delay(1);
        digitalWrite(_int, LOW);
        delay(1);
    }

    // Reset Pin Configuration
    if (_rst != -1) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, LOW);
        delay(10);
        digitalWrite(_rst, HIGH);
        delay(300);
    }

    memset(&data, 0, sizeof(data));

    data.version = i2c_read(0x15);
    delay(5);
    x = i2c_read_continuous(0xA7, data.versionInfo, 3);
    if (x != 0) {
        Serial.print("Err reading data.versionInfo: ");
        Serial.println(x);
    }

    disable_auto_sleep();

    // Enable continuous gesture actions and double-click
    enable_double_click(0x07);


    /*  Untested
    i2c_write(0xEF, 30); // MotionSlAngle
    i2c_write(0xFA, 0X79); // IrqCtl
    i2c_write(0xFB, 5); // AutoReset
    i2c_write(0xFC, 10); // LongPressTime
    */
}

/*!
    @brief  update data obj
*/
void CST820::read_touch() {
    byte data_raw[8];
    int8_t x;
    do {
        x = i2c_read_continuous(0x01, data_raw, 6);   // -1 = err, 0 = good
    } while(x);

    data.gestureID = data_raw[0] & 0x0F;  // Gesture 
    data.points = data_raw[1];	   // number of touch points
    data.event = data_raw[2] >> 6;  // Event (0 = Down, 1 = Up, 2 = Contact)
    data.x = ((data_raw[2] & 0xF) << 8) + data_raw[3];
    data.y = ((data_raw[4] & 0xF) << 8) + data_raw[5];
}

/*
    Legacy call interface
*/
uint8_t CST820::getTouch(uint16_t *x, uint16_t *y, uint8_t *gesture) {
    read_touch();

    *gesture = data.gestureID;
    // if (!(*gesture == SlideUp || *gesture == SlideDown))
    // {
    //     *gesture = None;
    // }

    *x = data.x;
    *y = data.y;

    return data.points;
}

/*
    test for input
*/
bool CST820::available() {
    read_touch();
    return static_cast<bool>(data.points);
}


uint8_t CST820::i2c_read(uint8_t addr) {
    uint8_t rdData = 0;
    uint8_t rdDataCount;
    do {
        Wire.beginTransmission(I2C_ADDR_CST820);
        Wire.write(addr);
        Wire.endTransmission(false);  // Restart
        rdDataCount = Wire.requestFrom(I2C_ADDR_CST820, 1);
    } while (rdDataCount == 0);
    while (Wire.available()) {
        rdData = Wire.read();
    }
    return rdData;
}

int8_t CST820::i2c_read_continuous(uint8_t addr, uint8_t *data, uint32_t length) {
  Wire.beginTransmission(I2C_ADDR_CST820);
  Wire.write(addr);
  if ( Wire.endTransmission(true))
      return -1;
  Wire.requestFrom(I2C_ADDR_CST820, length);
  for (int i = 0; i < length; i++) {
    *data++ = Wire.read();
  }
  return 0;
}

int8_t CST820::i2c_write(uint8_t addr, uint8_t data) {
  Wire.beginTransmission(I2C_ADDR_CST820);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
  return Wire.endTransmission(true);
}

int8_t CST820::i2c_write_continuous(uint8_t addr, const uint8_t *data, uint32_t length) {
  Wire.beginTransmission(I2C_ADDR_CST820);
  Wire.write(addr);
  for (int i = 0; i < length; i++) {
    Wire.write(*data++);
  }
  return Wire.endTransmission(true);
}

/*
   - Bit 0: EnDClick (enable double-click)
   - Bit 1: EnConUD (enable continuous up/down swipe)
   - Bit 2: EnConLR (enable continuous left/right swipe)
*/
int8_t CST820::enable_double_click(byte enable) {    // default 0x01
  // byte enableDoubleTap = 0x01;  // Set EnDClick (bit 0) to enable double-tap
  return i2c_write(0xEC, enable & 0x07);
}

/*!
    Disable double-tap
*/
int8_t CST820::disable_double_click(void) {
  byte doubleTap = 0x00;  // 0 value enables auto sleep
  return i2c_write(0xEC, doubleTap);
}

/*!
    @brief  Disable auto sleep mode
*/
int8_t CST820::disable_auto_sleep(void) {
  byte disableAutoSleep = 0x01;  // 0x01 value disables auto sleep
  return i2c_write(0xFE, disableAutoSleep);
}

/*!
    @brief  Enable auto sleep mode
*/
int8_t CST820::enable_auto_sleep(void) {
  byte enableAutoSleep = 0x00;  // 0 value enables auto sleep
  return i2c_write(0xFE, enableAutoSleep);
}


/*!
    @brief  Set the auto sleep time
    @param  seconds Time in seconds (1-255) before entering standby mode after inactivity
*/
int8_t CST820::set_auto_sleep_time(int seconds) {
  if (seconds < 1) {
    seconds = 1;  // Enforce minimum value of 1 second
  } else if (seconds > 255) {
    seconds = 255;  // Enforce maximum value of 255 seconds
  }

  byte sleepTime = static_cast<byte>(seconds);  // Convert int to byte
  return i2c_write(0xF9, sleepTime);
}

/*!
    @brief  get the gesture event name
*/
String CST820::gesture() {
  switch (data.gestureID) {
    case None:
      return "NONE";
      break;
    case SlideDown:
      return "SWIPE DOWN";
      break;
    case SlideUp:
      return "SWIPE UP";
      break;
    case SlideLeft:
      return "SWIPE LEFT";
      break;
    case SlideRight:
      return "SWIPE RIGHT";
      break;
    case SingleTap:
      return "SINGLE CLICK";
      break;
    case DoubleTap:
      return "DOUBLE CLICK";
      break;
    case LongPress:
      return "LONG PRESS";
      break;
    default:
      return (String)data.gestureID;
      break;
  }
}

String CST820::event_type() {
    return EVENT_TYPES[data.event];
}

