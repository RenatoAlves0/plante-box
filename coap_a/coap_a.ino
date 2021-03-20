#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>
#include <HTTPClient.h>
#include <time.h>
#include <math.h>

#define tempo 2500

#define ssid "..."
#define password "windows10mobile1"
#define cliente_id "plante_box"
#define servidor IPAddress(192, 168, 0, 6)
#define porta 5683

WiFiUDP udp;
Coap coap(udp);
HTTPClient http;

void conectarWiFi();
void put_coap();
void obterTempoAtual();

void setup()
{
    Serial.begin(115200);
    conectarWiFi();

    coap.start();
    obterTempoAtual();
}

void obterTempoAtual()
{
    http.begin("http://192.168.0.6:8080/");
    int httpCode = http.GET();
    clock_t ini_req = clock();

    if (httpCode > 0 && httpCode == HTTP_CODE_OK)
    {
        double tempo_req = ((double)(clock() - ini_req) / CLOCKS_PER_SEC) / 2;
        String t = http.getString();
        Serial.println(t);

        double tempo_segundo, tempo_micro_segundo, t_s_aux, t_m_s_aux;
        sscanf(t.substring(0, 10).c_str(), "%lf", &tempo_segundo);
        sscanf(t.substring(11).c_str(), "%lf", &tempo_micro_segundo);
        tempo_micro_segundo = tempo_micro_segundo * 1000;
        Serial.println("tempo_segundo:");
        Serial.println(tempo_segundo);
        Serial.println("");

        t_m_s_aux = modf(tempo_req, &t_s_aux);
        tempo_micro_segundo = tempo_micro_segundo + (t_m_s_aux * 1000 * 1000);
        tempo_segundo = tempo_segundo + t_s_aux;
        Serial.println("tempo_segundo:");
        Serial.println(tempo_segundo);
        Serial.println("");

        t_m_s_aux = modf(tempo_micro_segundo / (1000 * 1000), &t_s_aux);
        tempo_micro_segundo = t_m_s_aux * 1000 * 1000;
        tempo_segundo = tempo_segundo + t_s_aux;
        Serial.println("tempo_segundo:");
        Serial.println(tempo_segundo);
        Serial.println("");

        struct timeval now = {.tv_sec = (int)tempo_segundo, .tv_usec = (int)tempo_micro_segundo};
        settimeofday(&now, NULL);
        put_coap();
    }
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

void put_coap()
{
    struct timeval agora;
    char json[1024];
    clock_t ini = clock();

    while (((double)(clock() - ini) / CLOCKS_PER_SEC) <= 61)
    {
        Serial.println("CLOCK");
        Serial.println((double)(clock() - ini) / CLOCKS_PER_SEC);

        // delay(1000);
        // delay(750);
        // delay(500);
        // delay(250);
        delay(100);
        gettimeofday(&agora, NULL);
        sprintf(json, "{\"s\":%u, \"us\":%u, \"a\":\"123abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\"}", agora.tv_sec, agora.tv_usec);
        int msgid = coap.put(servidor, porta, "0", json);
    }
    sprintf(json, "{\"fim\":\"1\"}");
    int msgid = coap.put(servidor, porta, "0", json);
}

void loop() {}