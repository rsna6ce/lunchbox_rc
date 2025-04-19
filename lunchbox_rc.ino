#include <stdint.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <time.h>
#include "mycon.h"
#include <esp_task_wdt.h>
#include "wps_example.h"
#include "SPIFFSIni.h"

static MyconReceiver MyconRecv;

const int pin_l_f = 16;
const int pin_l_b = 17;
const int pin_r_f = 21;
const int pin_r_b = 22;
const int pinLED = 2;

const int pwm_ch_r_f = 1;
const int pwm_ch_r_b = 2;
const int pwm_ch_l_f = 3;
const int pwm_ch_l_b = 4;
const int pwm_freq = 30;
const int pwm_bit = 8;
const int pwm_max = (1 << pwm_bit);
static int pwm_percent_L1 = 30;
static int pwm_percent_L2 = 60;
static int pwm_percent_L3 = 100;
static int pwm_percent_R1 = 30;
static int pwm_percent_R2 = 60;
static int pwm_percent_R3 = 100;
static int turn_percent_rate_I1 = 30;
static int turn_percent_rate_I2 = 40;
static int turn_percent_rate_I3 = 50;
static int turn_percent_rate_O1 = 130;
static int turn_percent_rate_O2 = 130;
static int turn_percent_rate_O3 = 100;

WebServer server(80);
static String current_ipaddr = "";

//3 seconds WDT
#define WDT_TIMEOUT 3

const char* ssid = "";
const char* password = "";
#define WIFI_TIMEOUT 8


// Serial.readStringUntil do now work in ESP32...
const int ssid_pass_buff_len = 64;
char ssid_buff[ssid_pass_buff_len]={};
char pass_buff[ssid_pass_buff_len]={};
String SerialReasStringUntilCRLF() {
    Serial.setTimeout(100);
    String ret = "";
    String temp_str = "";
    while (true) {
        if (Serial.available() > 0) {
            temp_str = Serial.readString();
            if (temp_str.endsWith("\n") || temp_str.endsWith("\r")) {
                temp_str.replace("\n", "");
                temp_str.replace("\r", "");
                ret += temp_str;
                break;
            } else {
                ret += temp_str;
            }
        }
    }
    return ret;
}

void input_ssid_pass(char* ssid, char* pass) {
    while(true) {
        Serial.println("input SSID and press Enter.");
        String temp_ssid = SerialReasStringUntilCRLF();

        Serial.println("input password and press Enter.");
        String temp_pass = SerialReasStringUntilCRLF();

        Serial.println("SSID: " + temp_ssid + "\r\n" +
                       "pass: " + temp_pass + "\r\n" +
                       "OK? input yes or no and Enter key.");
        String temp_ret = SerialReasStringUntilCRLF();
        if (temp_ret == "yes") {
            temp_ssid.toCharArray(ssid, ssid_pass_buff_len);
            temp_pass.toCharArray(pass, ssid_pass_buff_len);
            break;
        }
    }
    return;
}

static int led_blink=0;
void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("hello. lunchbox.");
    SPIFFSIni config("/config.ini", true);

    // setup pins
    pinMode(pin_l_f, OUTPUT);
    pinMode(pin_l_b, OUTPUT);
    pinMode(pin_r_f, OUTPUT);
    pinMode(pin_r_b, OUTPUT);
    digitalWrite(pin_l_f, LOW);
    digitalWrite(pin_l_b, LOW);
    digitalWrite(pin_r_f, LOW);
    digitalWrite(pin_r_b, LOW);
    // setup pwm
    ledcSetup(pwm_ch_l_f, pwm_freq, pwm_bit);
    ledcSetup(pwm_ch_l_b, pwm_freq, pwm_bit);
    ledcSetup(pwm_ch_r_f, pwm_freq, pwm_bit);
    ledcSetup(pwm_ch_r_b, pwm_freq, pwm_bit);
    ledcAttachPin(pin_l_f, pwm_ch_l_f);
    ledcAttachPin(pin_l_b, pwm_ch_l_b);
    ledcAttachPin(pin_r_f, pwm_ch_r_f);
    ledcAttachPin(pin_r_b, pwm_ch_r_b);
    ledcWrite(pwm_ch_l_f, 0);
    ledcWrite(pwm_ch_l_b, 0);
    ledcWrite(pwm_ch_r_f, 0);
    ledcWrite(pwm_ch_r_b, 0);

    Serial.println("To specify the SSID, press the y key within 3 seconds.");
    delay(3*1000);
    if (Serial.available() > 0) {
        int inmyte = Serial.read();
        if (inmyte == 'y') {
            String frush_str = Serial.readString(); // flush buffer
            input_ssid_pass(ssid_buff, pass_buff);
            ssid = ssid_buff;
            password = pass_buff;
            config.write("SSID", String(ssid_buff));
            config.write("PASS", String(pass_buff));
        }
    }

    if (config.exist("SSID")) {
        String SSID = config.read("SSID");
        SSID.toCharArray(ssid_buff, ssid_pass_buff_len);
        ssid = ssid_buff;
        Serial.print("config SSID ");
        Serial.println(SSID);
    }
    if (config.exist("PASS")) {
        String PASS = config.read("PASS");
        PASS.toCharArray(pass_buff, ssid_pass_buff_len);
        password = pass_buff;
        Serial.print("config PASS ");
        Serial.println(PASS);
    }

    delay(1*1000);
    Serial.println("WiFi.begin");
    if (strlen(ssid)>0 && strlen(password)>0) {
        // specific ssid
        WiFi.begin(ssid, password);
    } else {
        // ssid last connected
        WiFi.begin();
    }
    int wifi_status = WL_DISCONNECTED;
    pinMode(0, INPUT_PULLUP);
    if (digitalRead(0) == LOW) {
        Serial.println("WiFi canceled");
        delay(1*1000);
    } else {
        for (int i=0; (i<WIFI_TIMEOUT*2)&&(wifi_status!= WL_CONNECTED); i++) {
            Serial.print(".");
            wifi_status = WiFi.status();
            digitalWrite(pinLED, led_blink);
            led_blink ^= 1;
            delay(500);
        }
    }
    digitalWrite(pinLED, LOW);
    if (wifi_status == WL_CONNECTED) {
        delay(1*1000);
        Serial.println("wifi connected.");
        Serial.println(WiFi.SSID());
        current_ipaddr = WiFi.localIP().toString();
        Serial.println(current_ipaddr);
        Serial.println(WiFi.macAddress());
        digitalWrite(pinLED, HIGH);
        delay(1*1000);
    } else {
        // try WPS connection
        Serial.println("Failed to connect");
        Serial.println("Starting WPS");
        delay(1*1000);
        WiFi.disconnect();
        WiFi.onEvent(WiFiEvent);
        WiFi.mode(WIFI_MODE_STA);
        wpsInitConfig();
        wpsStart();
        for (int i=0; (i<WIFI_TIMEOUT/2)&&(!wps_success); i++) {
            Serial.print(".");
            // wps_success is updated in callback
            digitalWrite(pinLED, led_blink);
            led_blink ^= 1;
            delay(2000);
        }
        if (wps_success) {
            Serial.println("wps_success!! and reset config SSID/PASS.");
            config.write("SSID", "");
            config.write("PASS", "");
        }
        delay(2*1000);
        digitalWrite(pinLED, LOW);
        esp_restart();
    }
    MyconRecv.begin(MYCON_UDP_PORT);
    Serial.println("start finished");
    delay(100);

    // load config
    Serial.println("loading config.ini ...");
    if (config.exist("pwm_percent_L1")) {
        pwm_percent_L1 = config.read("pwm_percent_L1").toInt();
        Serial.println("config read pwm_percent_L1");
    }
    if (config.exist("pwm_percent_L2")) {
        pwm_percent_L2 = config.read("pwm_percent_L2").toInt();
        Serial.println("config read pwm_percent_L2");
    }
    if (config.exist("pwm_percent_L3")) {
        pwm_percent_L3 = config.read("pwm_percent_L3").toInt();
        Serial.println("config read pwm_percent_L3");
    }
    if (config.exist("pwm_percent_R1")) {
        pwm_percent_R1 = config.read("pwm_percent_R1").toInt();
        Serial.println("config read pwm_percent_R1");
    }
    if (config.exist("pwm_percent_R2")) {
        pwm_percent_R2 = config.read("pwm_percent_R2").toInt();
        Serial.println("config read pwm_percent_R2");
    }
    if (config.exist("pwm_percent_R3")) {
        pwm_percent_R3 = config.read("pwm_percent_R3").toInt();
        Serial.println("config read pwm_percent_R3");
    }
    if (config.exist("turn_percent_rate_I1")) {
        turn_percent_rate_I1 = config.read("turn_percent_rate_I1").toInt();
        Serial.println("config read turn_percent_rate_I1");
    }
    if (config.exist("turn_percent_rate_I2")) {
        turn_percent_rate_I2 = config.read("turn_percent_rate_I2").toInt();
        Serial.println("config read turn_percent_rate_I2");
    }
    if (config.exist("turn_percent_rate_I3")) {
        turn_percent_rate_I3 = config.read("turn_percent_rate_I3").toInt();
        Serial.println("config read turn_percent_rate_I3");
    }
    if (config.exist("turn_percent_rate_O1")) {
        turn_percent_rate_O1 = config.read("turn_percent_rate_O1").toInt();
        Serial.println("config read turn_percent_rate_O1");
    }
    if (config.exist("turn_percent_rate_O2")) {
        turn_percent_rate_O2 = config.read("turn_percent_rate_O2").toInt();
        Serial.println("config read turn_percent_rate_O2");
    }
    if (config.exist("turn_percent_rate_O3")) {
        turn_percent_rate_O3 = config.read("turn_percent_rate_O3").toInt();
        Serial.println("config read turn_percent_rate_O3");
    }
    Serial.println("pwm_percent_L1:" + String(pwm_percent_L1));
    Serial.println("pwm_percent_L2:" + String(pwm_percent_L2));
    Serial.println("pwm_percent_L3:" + String(pwm_percent_L3));
    Serial.println("pwm_percent_R1:" + String(pwm_percent_R1));
    Serial.println("pwm_percent_R2:" + String(pwm_percent_R2));
    Serial.println("pwm_percent_R3:" + String(pwm_percent_R3));
    Serial.println("turn_percent_rate_I1:" + String(turn_percent_rate_I1));
    Serial.println("turn_percent_rate_I2:" + String(turn_percent_rate_I2));
    Serial.println("turn_percent_rate_I3:" + String(turn_percent_rate_I3));
    Serial.println("turn_percent_rate_O1:" + String(turn_percent_rate_O1));
    Serial.println("turn_percent_rate_O2:" + String(turn_percent_rate_O2));
    Serial.println("turn_percent_rate_O3:" + String(turn_percent_rate_O3));

    server.on("/", handleRoot);
    server.on("/api",handleApi);
    server.onNotFound(handleNotFound);
    server.begin();

    //enable panic so ESP32 restarts
    esp_task_wdt_init(WDT_TIMEOUT, true); 
    esp_task_wdt_add(NULL);
}

void handleRoot() {
    #include "config.html.h"
    config_html.replace("{{CURRENT_IPADDR}}", current_ipaddr);
    config_html.replace("{{pwm_percent_L1}}", String(pwm_percent_L1));
    config_html.replace("{{pwm_percent_L2}}", String(pwm_percent_L2));
    config_html.replace("{{pwm_percent_L3}}", String(pwm_percent_L3));
    config_html.replace("{{pwm_percent_R1}}", String(pwm_percent_R1));
    config_html.replace("{{pwm_percent_R2}}", String(pwm_percent_R2));
    config_html.replace("{{pwm_percent_R3}}", String(pwm_percent_R3));
    config_html.replace("{{turn_percent_rate_I1}}", String(turn_percent_rate_I1));
    config_html.replace("{{turn_percent_rate_I2}}", String(turn_percent_rate_I2));
    config_html.replace("{{turn_percent_rate_I3}}", String(turn_percent_rate_I3));
    config_html.replace("{{turn_percent_rate_O1}}", String(turn_percent_rate_O1));
    config_html.replace("{{turn_percent_rate_O2}}", String(turn_percent_rate_O2));
    config_html.replace("{{turn_percent_rate_O3}}", String(turn_percent_rate_O3));

    SPIFFSIni config("/config.ini", true);
    config_html.replace("{{SSID}}", String(config.read("SSID")));
    config_html.replace("{{PASS}}", String(config.read("PASS")));

    server.send(200, "text/HTML", config_html);
}

void handleApi() {
    String ev_str = server.arg("ev");
    String res = "ERROR: invalid command.";
    if(ev_str == "config") {
        String name_str = server.arg("name");
        String val_str = server.arg("val");
        SPIFFSIni config("/config.ini");
        if (name_str == "pwm_percent_L1") {
            pwm_percent_L1 = val_str.toInt();
            res = "OK pwm_percent_L1[%] = " + String(pwm_percent_L1);
            bool ret = config.write("pwm_percent_L1", String(pwm_percent_L1));
            Serial.println("config write pwm_percent_L1");
            Serial.println(ret);
        } else if (name_str == "pwm_percent_L2") {
            pwm_percent_L2 = val_str.toInt();
            res = "OK pwm_percent_L2[%] = " + String(pwm_percent_L2);
            config.write("pwm_percent_L2", String(pwm_percent_L2));
            Serial.println("config write pwm_percent_L2");
        } else if (name_str == "pwm_percent_L3") {
            pwm_percent_L3 = val_str.toInt();
            res = "OK pwm_percent_L3[%] = " + String(pwm_percent_L3);
            config.write("pwm_percent_L3", String(pwm_percent_L3));
            Serial.println("config write pwm_percent_L3");
        } else if (name_str == "pwm_percent_R1") {
            pwm_percent_R1 = val_str.toInt();
            res = "OK pwm_percent_R1[%] = " + String(pwm_percent_R1);
            config.write("pwm_percent_R1", String(pwm_percent_R1));
            Serial.println("config write pwm_percent_R1");
        } else if (name_str == "pwm_percent_R2") {
            pwm_percent_R2 = val_str.toInt();
            res = "OK pwm_percent_R2[%] = " + String(pwm_percent_R2);
            config.write("pwm_percent_R2", String(pwm_percent_R2));
            Serial.println("config write pwm_percent_R2");
        } else if (name_str == "pwm_percent_R3") {
            pwm_percent_R3 = val_str.toInt();
            res = "OK pwm_percent_R3[%] = " + String(pwm_percent_R3);
            config.write("pwm_percent_R3", String(pwm_percent_R3));
            Serial.println("config write pwm_percent_R3");
        } else if (name_str == "turn_percent_rate_I1") {
            turn_percent_rate_I1 = val_str.toInt();
            res = "OK turn_percent_rate_I1[%] = " + String(turn_percent_rate_I1);
            config.write("turn_percent_rate_I1", String(turn_percent_rate_I1));
            Serial.println("config write turn_percent_rate_I1");
        } else if (name_str == "turn_percent_rate_I2") {
            turn_percent_rate_I2 = val_str.toInt();
            res = "OK turn_percent_rate_I2[%] = " + String(turn_percent_rate_I2);
            config.write("turn_percent_rate_I2", String(turn_percent_rate_I2));
            Serial.println("config write turn_percent_rate_I2");
        } else if (name_str == "turn_percent_rate_I3") {
            turn_percent_rate_I3 = val_str.toInt();
            res = "OK turn_percent_rate_I3[%] = " + String(turn_percent_rate_I3);
            config.write("turn_percent_rate_I3", String(turn_percent_rate_I3));
            Serial.println("config write turn_percent_rate_I3");
        } else if (name_str == "turn_percent_rate_O1") {
            turn_percent_rate_O1 = val_str.toInt();
            res = "OK turn_percent_rate_O1[%] = " + String(turn_percent_rate_O1);
            config.write("turn_percent_rate_O1", String(turn_percent_rate_O1));
            Serial.println("config write turn_percent_rate_O1");
        } else if (name_str == "turn_percent_rate_O2") {
            turn_percent_rate_O2 = val_str.toInt();
            res = "OK turn_percent_rate_O2[%] = " + String(turn_percent_rate_O2);
            config.write("turn_percent_rate_O2", String(turn_percent_rate_O2));
            Serial.println("config write turn_percent_rate_O2");
        } else if (name_str == "turn_percent_rate_O3") {
            turn_percent_rate_O3 = val_str.toInt();
            res = "OK turn_percent_rate_O3[%] = " + String(turn_percent_rate_O3);
            config.write("turn_percent_rate_O3", String(turn_percent_rate_O3));
            Serial.println("config write turn_percent_rate_O3");
        } else if (name_str == "SSID") {
            res = "OK SSID = " + val_str;
            config.write("SSID", String(val_str));
        } else if (name_str == "PASS") {
            res = "OK PASS = " + val_str;
            config.write("PASS", String(val_str));
        }
    }
    server.send(200, "text/HTML", res);
}

void handleNotFound() {
  server.send(404, "text/plain", "404 page not found.");
}

void motor_output(int output_l_f, int output_l_b, int output_r_f, int output_r_b) {
    //debug
    //Serial.println(String(output_l_f) + "," + String(output_l_b) + "," + String(output_r_f) + "," + String(output_r_b));
    //write
    ledcWrite(pwm_ch_l_f, (pwm_max * output_l_f) / 100);
    ledcWrite(pwm_ch_l_b, (pwm_max * output_l_b) / 100);
    ledcWrite(pwm_ch_r_f, (pwm_max * output_r_f) / 100);
    ledcWrite(pwm_ch_r_b, (pwm_max * output_r_b) / 100);
}

void loop() {
    // web i/f
    if (WiFi.status()==WL_CONNECTED) {
        server.handleClient();
    } else {
        Serial.println("ERROR: wifi disconnected!! restart...");
        esp_restart();
    }

    // input check
    bool input_forward = MyconRecv.is_key_down(key_Upward);
    bool input_backward = MyconRecv.is_key_down(key_Downward) & (!input_forward);
    bool input_left = MyconRecv.is_key_down(key_Left) | MyconRecv.is_key_down(key_D);
    bool input_right = (MyconRecv.is_key_down(key_Right) | MyconRecv.is_key_down(key_A)) & (!input_left);
    bool input_spin_left = MyconRecv.is_key_down(key_Left);
    bool input_spin_right = MyconRecv.is_key_down(key_Right) & (!input_spin_left);;

    int speed_level = 0;
    if (MyconRecv.is_key_down(key_L1) | MyconRecv.is_key_down(key_1)){ speed_level++; }
    if (MyconRecv.is_key_down(key_L2) | MyconRecv.is_key_down(key_2)){ speed_level++; }

    //TODO: この下未実装
    const int pwm_percent_Ls[] = {pwm_percent_L1, pwm_percent_L2, pwm_percent_L3};
    const int pwm_percent_Rs[] = {pwm_percent_R1, pwm_percent_R2, pwm_percent_R3};
    const int turn_percent_rate_Is[] = {turn_percent_rate_I1, turn_percent_rate_I2, turn_percent_rate_I3};
    const int turn_percent_rate_Os[] = {turn_percent_rate_O1, turn_percent_rate_O2, turn_percent_rate_O3};
    int pwm_percent_L = pwm_percent_Ls[speed_level];
    int pwm_percent_R = pwm_percent_Rs[speed_level];
    int turn_percent_rate_I = turn_percent_rate_Is[speed_level];
    int turn_percent_rate_O = turn_percent_rate_Os[speed_level];

    // calc speed
    if (input_forward && input_left) {
        // forward turn left
        //Serial.println("forward turn left");
        motor_output(
            pwm_percent_L * turn_percent_rate_I/100,
            0,
            pwm_percent_R * turn_percent_rate_O/100,
            0);
    } else if (input_forward && input_right) {
        // forward turn right
        //Serial.println("forward turn right");
        motor_output(
            pwm_percent_L * turn_percent_rate_O/100,
            0,
            pwm_percent_R * turn_percent_rate_I/100,
            0);
    } else if (input_forward){
        // forward
        //Serial.println("forward");
        motor_output(
            pwm_percent_L,
            0,
            pwm_percent_R,
            0);
    } else if (input_backward && input_left) {
        // backward turn left
        //Serial.println("backward turn left");
        motor_output(
            0,
            pwm_percent_L * turn_percent_rate_I/100,
            0,
            pwm_percent_R * turn_percent_rate_O/100);
    } else if (input_backward && input_right) {
        // backwardturn right
        //Serial.println("backwardturn right");
        motor_output(
            0,
            pwm_percent_L * turn_percent_rate_O/100,
            0,
            pwm_percent_R * turn_percent_rate_I/100);
    } else if (input_backward){
        // backward
        //Serial.println("backward");
        motor_output(
            0,
            pwm_percent_L,
            0,
            pwm_percent_R);
    } else if (input_spin_left) {
        // spin left
        //Serial.println("spin left");
        motor_output(
            0,
            pwm_percent_L,
            pwm_percent_R,
            0);
    } else if (input_spin_right) {
        // spin right
        //Serial.println("spin right");
        motor_output(
            pwm_percent_L,
            0,
            0,
            pwm_percent_R);
    } else {
        // stop
        //Serial.println("stop");
        motor_output(0, 0, 0, 0);
    }

    esp_task_wdt_reset();
    delay(3);
}
