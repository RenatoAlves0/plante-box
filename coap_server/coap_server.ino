#include <ESP8266WiFi.h>
#include "coap_server.h"

#define ssid "..."
#define password "windows10mobile1"

coapServer coap;

void conectarWiFi();
void callback_led(coapPacket &packet, IPAddress ip, int port, int obs);

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

void callback_led(coapPacket *packet, IPAddress ip, int port, int obs)
{
    Serial.println("");
    // Serial.println("tópico LED");
    Serial.print("obs: ");
    Serial.println(obs);

    char p[packet->payloadlen + 1];
    memcpy(p, packet->payload, packet->payloadlen);
    p[packet->payloadlen] = NULL;

    String message(p);

    if (message.equals("0"))
    {
        Serial.println("DESLIGAR Led");
        digitalWrite(2, HIGH); //desligado
    }
    else if (message.equals("1"))
    {
        Serial.println("LIGAR Led");
        digitalWrite(2, LOW); //ligado
    }

    char *led = (digitalRead(2) > 0) ? ((char *)"1") : ((char *)"0");

    if (obs == 1)
        coap.sendResponse(led);
    else
        coap.sendResponse(ip, port, led);
}

void setup()
{
    Serial.begin(115200);
    conectarWiFi();

    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH); //desligado

    coap.server(callback_led, "led");

    // coap.start();
    coap.start(5683);
}

void loop()
{
    coap.loop();
    // delay(1000);
}
