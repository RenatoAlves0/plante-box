#include <WiFi.h>
#include "coap_client.h"

coapClient coap;

const char *ssid = "...";
const char *password = "windows10mobile1";

IPAddress ip(192, 168, 0, 5);
int port = 5683;
int i = 0;

// Prototypes
void conectarWiFi();
void callback_response(coapPacket &packet, IPAddress ip, int port);

// Métodos
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

void callback_response(coapPacket &packet, IPAddress ip, int port)
{
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = NULL;

    //response from coap server
    if (packet.type == 3 && packet.code == 0)
        Serial.println("ping ok");

    Serial.println(p);
}

void setup()
{
    Serial.begin(115200);
    conectarWiFi();

    coap.response(callback_response);
    coap.start();

    // int msgid = coap.observe(ip, port, "luz", 0);
    // int msgid = coap.get(ip, port, "luz");
    // int msgid = coap.observecancel(ip, port, "resoucename");
}

void loop()
{
    coap.observe(ip, port, "luz", 0);

    delay(1000);

    Serial.println("Tô vivo");
    coap.loop();

    //get request
    // int msgid = coap.get(ip,port,"hello");

    //put request
    //arguments server ip address,default port,resource name, payload,payloadlength
    //int msgid =coap.put(ip,port,"resourcename","0",strlen("0"));

    //post request
    //arguments server ip address,default port,resource name, payload,payloadlength
    //int msgid =coap.post(ip,port,"resourcename","0",strlen("0"));

    //delete request
    //int msgid = coap.delet(ip,port,"resourcename");

    //ping
    //int msgid=coap.ping(ip,port);

    //observe
    // int msgid = coap.observe(ip, port, "luz", 0);

    // state = coap.loop();
    // Serial.println(state);
    // if (state == true)
    //     i = i + 1;

    // Serial.print("i=");
    // Serial.println(i);
    // if (i == 3)
    // {
    //     Serial.println("cancel observe");
    //     coap.observeCancel(ip, port, "luz");
    // }

    // Serial.println(msgid);
}