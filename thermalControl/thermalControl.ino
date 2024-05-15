#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define POT_PIN 34
#define LED_PIN 13
#define L298_IN1 26
#define L298_IN2 27
#define L298_ENA 25

#define ONE_WIRE_BUS 14
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;

// PID variables
double Setpoint, Input, Output;
double Kp = 25, Ki = 0.2, Kd = 5;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

void setup() {
  // Khởi động Serial để debug
  Serial.begin(115200);

  // Khởi động OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Không thể khởi động OLED!"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(2); // Thiết lập cỡ chữ lớn hơn
  display.setTextColor(SSD1306_WHITE);

  // Cài đặt chân LED và L298
  pinMode(LED_PIN, OUTPUT);
  pinMode(L298_IN1, OUTPUT);
  pinMode(L298_IN2, OUTPUT);
  pinMode(L298_ENA, OUTPUT);

  // Thiết lập mặc định cho L298
  digitalWrite(L298_IN1, HIGH);
  digitalWrite(L298_IN2, LOW);

  // Khởi động cảm biến nhiệt độ DS18B20
  sensors.begin();
  if (!sensors.getAddress(insideThermometer, 0)) {
    Serial.println("Không tìm thấy cảm biến DS18B20!");
  }

  // Thiết lập PID
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, 255); // Giới hạn giá trị output từ 0 đến 255 (phù hợp với PWM)
}

void loop() {
  // Đọc giá trị từ biến trở và thiết lập setpoint
  int potValue = analogRead(POT_PIN);
  Setpoint = map(potValue, 1, 4095, 30, 50); // Giả sử nhiệt độ mong muốn từ 0 đến 100 độ C

  // Kiểm tra nếu đã đến lúc đọc giá trị nhiệt độ mới
 // Đọc nhiệt độ từ DS18B20
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempC(insideThermometer);

  // Cập nhật giá trị đầu vào cho PID
  Input = temperatureC;

  // Tính toán giá trị đầu ra của PID
  myPID.Compute();

  // Điều khiển độ sáng của bóng đèn bằng giá trị đầu ra của PID (PWM)
  analogWrite(L298_ENA, Output);

  // Hiển thị giá trị biến trở, giá trị PWM và nhiệt độ lên OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("SP: ");
  display.print(Setpoint, 1);
  display.setCursor(0, 20);
  display.print("PWM: ");
  display.print(Output, 1);
  display.setCursor(0, 40);
  display.print("T: ");
  display.print(temperatureC, 1); // Hiển thị nhiệt độ với 1 chữ số thập phân
  display.print(" C");
  display.display();

  // In giá trị biến trở, giá trị PWM và nhiệt độ lên Serial để debug
  Serial.print("Setpoint: ");
  Serial.print(Setpoint);
  Serial.print("  PWM Value: ");
  Serial.print(Output);
  Serial.print("  Temp: ");
  Serial.print(temperatureC);
  Serial.println(" C");

  // Tạm dừng một khoảng thời gian ngắn (có thể giảm thời gian delay này để chương trình nhanh hơn)
  // delay(10);
}
