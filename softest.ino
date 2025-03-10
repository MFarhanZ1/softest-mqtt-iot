#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Konfigurasi WiFi
const char *ssid = "wifi_ssid_name";         // Nama SSID WiFi
const char *password = "wifi_pass"; // Password WiFi

// Identitas Ruangan Target
const char *nama_ruangan = "lab-mm"; // Nama ruangan target

// Topic yang di-listen oleh ESP8266
String mqtt_topic_subscribe = "/tif/" + String(nama_ruangan); // MQTT topic subscribe

// Konfigurasi terhadap MQTT Broker
const char *mqtt_broker = "mqtt_broker";      // Alamat IP MQTT broker
const char *mqtt_username = "mqtt_username";        // MQTT username untuk autentikasi
const char *mqtt_password = "mqtt_pass"; // MQTT password untuk autentikasi
const int mqtt_port = 1883;                  // Port MQTT broker

const int debugMqttLED = D7; // Pin untuk indikator status koneksi dengan MQTT Broker
const int debugWifiLED = D3; // Pin untuk indikator status koneksi dengan WiFi
const int lampuUtama = D2; // Pin untuk lampu utama

// Inisisialisasi variabel objek global library
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// Pemanggilan fungsi utama dari program ini, dari konektivitas ke WiFi hingga koneksi ke MQTT Broker
void connectToWiFi();
void connectToMQTTBroker();
void mqttCallback(char *topic, byte *payload, unsigned int length);

void setup()
{
    // Inisialisasi Serial Monitor untuk melihat print statement dan prosesdebugging
    Serial.begin(115200);

    // Inisialisasi pin lampu-lampu sebagai OUTPUT
    pinMode(lampuUtama, OUTPUT);
    pinMode(debugWifiLED, OUTPUT);
    pinMode(debugMqttLED, OUTPUT);

    // Inisialisasi koneksi ke WiFi dan MQTT Broker
    connectToWiFi();    
    mqtt_client.setServer(mqtt_broker, mqtt_port);
    mqtt_client.setCallback(mqttCallback);    
    connectToMQTTBroker();
}

void connectToWiFi()
{      
    // Melakukan koneksi ke WiFi
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        // Print statement untuk memberi tahu progress uji coba konektivitas wifi
        Serial.print("#--Connecting--# ");
        delay(500);

        // Jika belum terkoneksi ke WiFi, maka LED indikator masih akan mati
        digitalWrite(debugWifiLED, LOW);
    }

    // Melihat log informasi wifi yang akan dicoba dikoneksikan
    Serial.println("\n[INFO] Dibawah ini merupakan kredensial WiFi yang dihubungkan...");
    Serial.println("[INFO] WiFi SSID: " + String(ssid));
    Serial.println("[INFO] WiFi Password: " + String(password));

    // Print statement untuk memberi tahu bahwa wifi sudah berhasil dikoneksikan
    Serial.println("[INFO] Yeay, berhasil terkoneksi ke WiFi!");
    Serial.println("---------------------------");

    // Jika sudah terkoneksi ke WiFi, maka LED indikator status konektivitas wifi akan menyala selama 1,5 detik
    digitalWrite(debugWifiLED, HIGH);
    delay(1500);
    digitalWrite(debugWifiLED, LOW);
}

void connectToMQTTBroker()
{
    // Melihat log informasi MQTT Broker yang akan dicoba dikoneksikan
    Serial.println("[INFO] Mencoba menghubungkan ke MQTT Broker sesuai kredensial dibawah ini...");
    Serial.println("[INFO] MQTT Broker: " + String(mqtt_broker));
    Serial.println("[INFO] MQTT Port: " + String(mqtt_port));
    Serial.println("[INFO] MQTT Username: " + String(mqtt_username));
    Serial.println("[INFO] MQTT Password: " + String(mqtt_password));
    Serial.println("[INFO] MQTT Topic Subscribe: " + mqtt_topic_subscribe);

    while (!mqtt_client.connected())
    {
        // Melakukan pembuatan client ID yang unik untuk ESP8266, dan melihat client ID yang akan digunakan
        String client_id = "esp8266-client-" + String(WiFi.macAddress());
        Serial.printf("[INFO] Mencoba menghubungkan ke MQTT Broker dengan client ID: %s.....\n", client_id.c_str());

        // Jika gagal terkoneksi ke MQTT Broker, maka akan mencoba kembali setiap 5 detik sekali
        if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password))
        {
            // Jika berhasil terkoneksi ke MQTT Broker, maka LED indikator status konektivitas MQTT Broker akan menyala selama 1,5 detik
            Serial.println("[INFO] Yeay, berhasil terkoneksi ke MQTT Broker!");
            digitalWrite(debugMqttLED, HIGH);
            delay(1500);
            digitalWrite(debugMqttLED, LOW);

            // Melakukan subscribe pada topic yang di-listen oleh ESP8266 tadi
            mqtt_client.subscribe(mqtt_topic_subscribe.c_str()); // Menggunakan c_str() untuk mengonversi String ke const char*

            // Print statement untuk memberi tahu bahwa ESP8266 sudah berhasil subscribe ke topic yang ditentukan di-awal
            Serial.printf("[INFO] Berhasil subscribe ke topic: %s\n", mqtt_topic_subscribe.c_str());
            Serial.println("---------------------------");
            Serial.println("[INFO] Yihiw, ESP8266 Ini Sekarang Siap Menerima Pesan...");
            Serial.println("---------------------------");
        }
        else
        {
            // Print statement untuk memberi tahu bahwa gagal terkoneksi ke MQTT Broker
            Serial.print("[INFO] Gagal terkoneksi ke MQTT Broker, rc=");
            Serial.print(mqtt_client.state());
            Serial.println("[INFO] Pencobaan koneksi ke MQTT Broker akan dilakukan setiap 5 detik sekali...");
            Serial.println("---------------------------");           
            delay(5000);
            digitalWrite(debugMqttLED, LOW);
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    // Melihat log informasi notif pesan MQTT masuk dan isi pesan MQTT yang diterima oleh ESP8266
    Serial.print("[INFO] Ada pesan MQTT masuk di-topik: ");
    Serial.println(topic);
    Serial.print("[INFO] Isi Pesan MQTT yang diterima oleh ESP8266: ");

    // Mengonversi payload pesan MQTT menjadi String dan menampilkannya
    String msg;
    for (unsigned int i = 0; i < length; i++)
    {
        msg += (char)payload[i];
    }
    Serial.println(msg);

    // Melakukan pengecekan isi pesan MQTT yang diterima oleh ESP8266
    if (msg == "ON")
    {
        // Jika pesan MQTT yang diterima oleh ESP8266 adalah "ON", maka akan menyalakan lampu utama
        Serial.println("[INFO] Menyalakan lampuUtama");
        digitalWrite(lampuUtama, HIGH); // Log Secara Fisik, Turn lampuUtama ON    
    }
    else if (msg == "OFF")
    {
        // Jika pesan MQTT yang diterima oleh ESP8266 adalah "OFF", maka akan mematikan lampu utama
        Serial.println("[INFO] Mematikan lampuUtama");
        digitalWrite(lampuUtama, LOW); // Log Secara Fisik, Turn lampuUtama OFF
    }

    Serial.println("---------------------------");
}

void loop()
{
    if (!mqtt_client.connected())
    {
        connectToMQTTBroker();
    }
    mqtt_client.loop();
}