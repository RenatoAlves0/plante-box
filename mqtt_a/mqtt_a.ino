#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <HTTPClient.h>

#define tempo 2500

#define ssid "..."
#define password "windows10mobile1"
#define cliente_id "plante_box"
#define servidor IPAddress(192, 168, 0, 6)
#define porta 1883

WiFiClient plante_box;
PubSubClient cliente(plante_box);
HTTPClient http;

void conectarWiFi();
void conectarMQTT();
void pub_mqtt();
void obterTempoAtual();

void setup()
{
    Serial.begin(115200);
    conectarWiFi();

    cliente.setKeepAlive(18000); //5h
    cliente.setServer(servidor, porta);
    conectarMQTT();
    obterTempoAtual();

    //struct timeval now = {.tv_sec = 1616001960, .tv_usec = 424 * 1000};
    //settimeofday(&now, NULL);
}

void obterTempoAtual()
{
    Serial.print("[HTTP] begin...\n");
    http.begin("http://192.168.0.6:8080/");

    Serial.print("[HTTP] GET...\n");
    int httpCode = http.GET();

    if (httpCode > 0)
    {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        if (httpCode == HTTP_CODE_OK)
        {
            String t = http.getString();
            Serial.println(t);

            int s, u;
            sscanf(t.substring(0, 10).c_str(), "%d", &s);
            sscanf(t.substring(11).c_str(), "%d", &u);

            struct timeval now = {.tv_sec = s, .tv_usec = u * 1000};
            settimeofday(&now, NULL);
        }
    }
    else
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
}

void conectarWiFi()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("!!! Conectando-se ao Wi-Fi !!!");
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED)
        {
            Serial.print(".");
            delay(500);
        }
        Serial.println();
        Serial.print("Conectado a '");
        Serial.print(ssid);
        Serial.println("'");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    }
}

void conectarMQTT()
{
    while (!cliente.connected())
    {
        Serial.println("!!! Conectando-se ao cliente !!!");
        if (cliente.connect(cliente_id))
        {
            Serial.print("Conectado a '");
            Serial.print(servidor);
            Serial.println("'");
            return;
        }
    }
}

void pub_mqtt()
{
    struct timeval agora;
    char json[1024];
    char dados[64] = "abc";

    while (true)
    {
        delay(1000);
        // delay(750);
        // delay(500);
        // delay(250);
        gettimeofday(&agora, NULL);
        sprintf(json, "{\"dados\":%u, \"envio_s\":%u, \"envio_us\":%u}", dados, agora.tv_sec, agora.tv_usec);
        cliente.publish("0", json);
    }
}

void loop()
{
    // struct timeval agora;
    // gettimeofday(&agora, NULL);
    // Serial.println("TEMPO");
    // Serial.println(agora.tv_sec);
    // Serial.println(agora.tv_usec);
    // Serial.println("");

    // conectarWiFi();
    // conectarMQTT();
    // delay(tempo);
    // cliente.loop();
}