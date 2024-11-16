#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "AndroidAP";
const char* password = "123456789";

WebServer server(80);

int16_t acc_x, acc_y, acc_z;
int16_t gyro_pitch, gyro_roll, gyro_yaw;
int64_t ax_total, ay_total, az_total;
uint8_t gyro_address = 0x68;
int16_t temperature;
float cal_ax, cal_ay, cal_az;
int cal_num = 2000;

void setup() {
  Serial.begin(500000);
  Wire.setClock(400000);  // Initializing I2C at 400 KHz mode
  Wire.begin();
  delay(250);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);  // Serve the initial web page with calibration results and button
  server.on("/calibrate", HTTP_GET, handleCalibrate);  // Handle calibration when the button is pressed
  server.begin();
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String webpage = "<html><head><title>Accelerometer Calibration</title></head><body>";
  webpage += "<h1>Calibration Results:</h1>";
  webpage += "<p>X-axis: " + String(cal_ax) + "</p>";
  webpage += "<p>Y-axis: " + String(cal_ay) + "</p>";
  webpage += "<p>Z-axis: " + String(cal_az) + "</p>";
  webpage += "<button onclick=\"location.href='/calibrate'\">Recalibrate</button>";
  webpage += "</body></html>";
  server.send(200, "text/html", webpage);
}

void handleCalibrate() {
  calibrateAccelerometer();
  handleRoot();  // Redirect back to root to show updated results
}

void calibrateAccelerometer() {
  ax_total = 0;
  ay_total = 0;
  az_total = 0;
  for (int i = 0; i < cal_num; i++) {
    gyro_signalen();
    ax_total += acc_x;
    ay_total += acc_y;
    az_total += acc_z;
    delay(4);
  }
  cal_ax = ax_total / cal_num;
  cal_ay = ay_total / cal_num;
  cal_az = az_total / cal_num;
}



void gyro_setup(void){
  Wire.beginTransmission(gyro_address);                        //Start communication with the MPU-6050.
  Wire.write(0x6B);                                            //We want to write to the PWR_MGMT_1 register (6B hex).
  Wire.write(0x00);                                            //Set the register bits as 00000000 to activate the gyro.
  Wire.endTransmission();                                      //End the transmission with the gyro.

  Wire.beginTransmission(gyro_address);                        //Start communication with the MPU-6050.
  Wire.write(0x1B);                                            //We want to write to the GYRO_CONFIG register (1B hex).
  Wire.write(0x08);                                            //Set the register bits as 00001000 (500dps full scale).
  Wire.endTransmission();                                      //End the transmission with the gyro.

  Wire.beginTransmission(gyro_address);                        //Start communication with the MPU-6050.
  Wire.write(0x1C);                                            //We want to write to the ACCEL_CONFIG register (1A hex).
  Wire.write(0x10);                                            //Set the register bits as 00010000 (+/- 8g full scale range).
  Wire.endTransmission();                                      //End the transmission with the gyro.

  Wire.beginTransmission(gyro_address);                        //Start communication with the MPU-6050.
  Wire.write(0x1A);                                            //We want to write to the CONFIG register (1A hex).
  Wire.write(0x03);                                            //Set the register bits as 00000011 (Set Digital Low Pass Filter to ~43Hz).
  Wire.endTransmission();                                      //End the transmission with the gyro.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void gyro_signalen(void) {
  Wire.beginTransmission(gyro_address);                       //Start communication with the gyro.
  Wire.write(0x3B);                                           //Start reading @ register 43h and auto increment with every read.
  Wire.endTransmission();                                     //End the transmission.
  Wire.requestFrom(gyro_address, 14);                         //Request 14 bytes from the MPU 6050.
  acc_y = Wire.read() << 8 | Wire.read();                    //Add the low and high byte to the acc_x variable.
  acc_x = Wire.read() << 8 | Wire.read();                    //Add the low and high byte to the acc_y variable.
  acc_z = Wire.read() << 8 | Wire.read();                    //Add the low and high byte to the acc_z variable.
  temperature = Wire.read() << 8 | Wire.read();              //Add the low and high byte to the temperature variable.
  gyro_roll = Wire.read() << 8 | Wire.read();                //Read high and low part of the angular data.
  gyro_pitch = Wire.read() << 8 | Wire.read();               //Read high and low part of the angular data.
  gyro_yaw = Wire.read() << 8 | Wire.read();                 //Read high and low part of the angular data.
  gyro_pitch *= -1;                                            //Invert the direction of the axis.
  gyro_yaw *= -1;                                              //Invert the direction of the axis.

}

// Ensure to paste the `gyro_setup()` and `gyro_signalen()` functions here
