#define DEBUG
//#define DEBUG_WIFI

#include <WiFiEsp.h>
#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <U8glib.h>

#define AP_SSID "embB"
#define AP_PASS "embB1234"
#define SERVER_NAME "10.10.14.70"
#define SERVER_PORT 5000  
#define LOGID "SHW_ARD"
#define PASSWD "PASSWD"


#define WIFITX 9  // 9: TX --> ESP8266 RX
#define WIFIRX 10 // 10: RX --> ESP8266 TX
#define LED_BUILTIN_PIN 13

#define MOTOR_PIN 3
#define GAS_SENSOR_PIN A0

#define CMD_SIZE 50
#define ARR_CNT 5

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0); // I2C / TWI

bool timerIsrFlag = false;
int dailyOutputCount = 0; // 하루에 출력된 횟수를 저장할 변수

char sendId[10] = "SHW_ARD";
char sendBuf[CMD_SIZE];


unsigned int secCount;

bool updatTimeFlag = false;
typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
} DATETIME;
DATETIME dateTime = { 0, 0, 0, 12, 0, 0 };

SoftwareSerial wifiSerial(WIFIRX, WIFITX);
WiFiEspClient client;

char oledLine[17] = "";
char oledLine1[17] = "";
char oledLine2[17] = "";
char oledLine3[17] = "";
char oledLine4[17] = "";

void wifi_Setup();
int server_Connect();
void wifi_Init();
void timerIsr();
void clock_calc(DATETIME* dateTime);

void setup() {

    Serial.begin(9600);
    pinMode(MOTOR_PIN, OUTPUT);
    u8g.setColorIndex(1);
    u8g.setFont(u8g_font_unifont);
    displayDrawStr();

    pinMode(LED_BUILTIN_PIN, OUTPUT); //D13-내장LED

#ifdef DEBUG
    Serial.begin(115200); //DEBUG
#endif
    wifi_Setup();

    MsTimer2::set(1000, timerIsr); // 1000ms period
    MsTimer2::start();

}

void loop() {
    if (client.available()) {
        socketEvent();
    }

    if (timerIsrFlag) //1초에 한번씩 실행
    {
        timerIsrFlag = false;

        int gasValue = analogRead(GAS_SENSOR_PIN);

        // OLED에 가스 농도 출력
        //displayDrawStr();
        if (gasValue > 350) {
            sprintf(oledLine2, "Cigarette detection!!");
            dailyOutputCount++; // 가스가 감지될 때마다 하루의 출력 횟수 증가
        }
        else {
            oledLine2[0] = '\0'; // 가스 농도 문구 지우기
        }
        sprintf(oledLine4, "gasValue : %d", gasValue);

        // 모터 제어
        digitalWrite(MOTOR_PIN, gasValue > 350 ? HIGH : LOW);

        // OLED에 현재 가스 센서 값 출력
        displayDrawStr();

        if (!(secCount % 5)) //5초에 한번씩 실행
        {
            if (!client.connected()) {
                sprintf(oledLine3, "Server Down");
                displayDrawStr();
                server_Connect();
            }
        }

        if (updatTimeFlag)
        {
            client.print("[GETTIME]\n");
            updatTimeFlag = false;
        }
        // 시간을 OLED에 표시
        sprintf(oledLine1, "%02d/%02d,%02d:%02d:%02d", dateTime.month, dateTime.day,
            dateTime.hour, dateTime.min, dateTime.sec);

        if (dateTime.hour == 13 && dateTime.min == 48 && dateTime.sec == 00) {
            sprintf(oledLine3, "T Cigarette:%d", dailyOutputCount);

            char countStr[50];
            sprintf(countStr, "[LSB_LIN]GROUP1@SHIN@%d\n", dailyOutputCount);
            client.print(countStr);
            dailyOutputCount = 0;
        }

    }
}

void socketEvent()
{
    int i = 0;
    char* pToken;
    char* pArray[ARR_CNT] = { 0 };
    char recvBuf[CMD_SIZE] = { 0 };
    int len;

    sendBuf[0] = '\0';
    len = client.readBytesUntil('\n', recvBuf, CMD_SIZE);
    client.flush();
#ifdef DEBUG
    Serial.print("recv : ");
    Serial.print(recvBuf);
#endif
    pToken = strtok(recvBuf, "[@]");
    while (pToken != NULL)
    {
        pArray[i] = pToken;
        if (++i >= ARR_CNT)
            break;
        pToken = strtok(NULL, "[@]");
    }

    if (!strncmp(pArray[1], " New", 4)) // New Connected
    {
#ifdef DEBUG
        Serial.write('\n');
#endif

        updatTimeFlag = true;
        return;
    }
    else if (!strncmp(pArray[1], " Alr", 4)) //Already logged
    {
#ifdef DEBUG
        Serial.write('\n');
#endif
        client.stop();
        server_Connect();
        return;
    }
    else if (!strcmp(pArray[1], "MOTOR")) {
        if (!strcmp(pArray[2], "ON")) {
            digitalWrite(MOTOR_PIN, HIGH);
        }
        else if (!strcmp(pArray[2], "OFF")) {
            digitalWrite(MOTOR_PIN, LOW);
        }
        sprintf(sendBuf, "[%s]%s@%s\n", pArray[0], pArray[1], pArray[2]);
    }
    else if (!strcmp(pArray[1], "GAS")) {
        if (!strcmp(pArray[2], "ON")) {
            digitalWrite(MOTOR_PIN, HIGH);
        }
        else if (!strcmp(pArray[2], "OFF")) {
            digitalWrite(MOTOR_PIN, LOW);

        }
        sprintf(sendBuf, "[%s]%s@%s\n", pArray[0], pArray[1], pArray[2]);
    }
    else if (!strcmp(pArray[0], "GETTIME")) {  //GETTIME
        dateTime.year = (pArray[1][0] - 0x30) * 10 + pArray[1][1] - 0x30;
        dateTime.month = (pArray[1][3] - 0x30) * 10 + pArray[1][4] - 0x30;
        dateTime.day = (pArray[1][6] - 0x30) * 10 + pArray[1][7] - 0x30;
        dateTime.hour = (pArray[1][9] - 0x30) * 10 + pArray[1][10] - 0x30;
        dateTime.min = (pArray[1][12] - 0x30) * 10 + pArray[1][13] - 0x30;
        dateTime.sec = (pArray[1][15] - 0x30) * 10 + pArray[1][16] - 0x30;
#ifdef DEBUG
        sprintf(sendBuf, "\nTime %02d.%02d.%02d %02d:%02d:%02d\n\r",
            dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.min, dateTime.sec);
        Serial.println(sendBuf);
#endif
        return;
    }
    else
        return;

    client.write(sendBuf, strlen(sendBuf));
    client.flush();

#ifdef DEBUG
    Serial.print(", send : ");
    Serial.print(sendBuf);
#endif
}
void timerIsr()
{
    timerIsrFlag = true;
    secCount++;
    clock_calc(&dateTime);
}
void clock_calc(DATETIME* dateTime)
{
    int ret = 0;
    dateTime->sec++;          // increment second

    if (dateTime->sec >= 60)                              // if second = 60, second = 0
    {
        dateTime->sec = 0;
        dateTime->min++;

        if (dateTime->min >= 60)                          // if minute = 60, minute = 0
        {
            dateTime->min = 0;
            dateTime->hour++;                               // increment hour
            if (dateTime->hour == 24)
            {
                dateTime->hour = 0;
                updatTimeFlag = true;
            }
        }
    }
}

void wifi_Setup() {
    wifiSerial.begin(38400);
    wifi_Init();
    server_Connect();
}
void wifi_Init()
{
    do {
        WiFi.init(&wifiSerial);
        if (WiFi.status() == WL_NO_SHIELD) {
#ifdef DEBUG_WIFI
            Serial.println("WiFi shield not present");
#endif
        }
        else
            break;
    } while (1);

#ifdef DEBUG_WIFI
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(AP_SSID);
#endif
    while (WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) {
#ifdef DEBUG_WIFI
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(AP_SSID);
#endif
    }

#ifdef DEBUG_WIFI
    Serial.println("You're connected to the network");
    printWifiStatus();
#endif
}
int server_Connect()
{
#ifdef DEBUG_WIFI
    Serial.println("Starting connection to server...");
#endif

    if (client.connect(SERVER_NAME, SERVER_PORT)) {
#ifdef DEBUG_WIFI
        Serial.println("Connect to server");
#endif
        client.print("["LOGID":"PASSWD"]");
    }
    else
    {
#ifdef DEBUG_WIFI
        Serial.println("server connection failure");
#endif
    }
}
void printWifiStatus()
{
    // print the SSID of the network you're attached to

    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength
    long rssi = WiFi.RSSI();
    Serial.print("Signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}

void displayDrawStr() {
    u8g.firstPage();
    do {
        u8g.drawStr(0, 15, oledLine1);
        u8g.drawStr(0, 31, oledLine2);
        u8g.drawStr(0, 47, oledLine3);
        u8g.drawStr(0, 63, oledLine4);
    } while (u8g.nextPage());
}