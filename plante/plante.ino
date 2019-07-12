#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define clima 23
#define clima_vcc 22

#define luz 36
#define luz_vcc 32

#define umidade_solo 39
#define umidade_solo_vcc 25

#define chuva 34
#define chuva_vcc 27

#define aguar_ini 19
#define aguar_vcc 18

#define tempo_l 1500
#define tempo 5000

#define ssid "..."
#define password "windows10mobile1"
#define topico "plante_iot_dados(renalves.oli)"
#define section_id "plante_iot_id(renalves.oli)"
#define broker "iot.eclipse.org"
#define broker_port 1883

//Variáveis globais
WiFiClient plante_iot;
PubSubClient MQTT(plante_iot);
DHT dht(clima, DHT11);
float _temperatura, _umidade, _umidadeSolo, _luz, _chuva;

//Prototypes
void conectarWiFi();
void conectarMQTT();
void pub_mqtt();
void sub_mqtt();
void get_data_mqtt(char *topic, byte *payload, unsigned int length);

void setup()
{
    Serial.begin(115200);
    conectarWiFi();

    MQTT.setServer(broker, broker_port);
    MQTT.setCallback(get_data_mqtt);
    conectarMQTT();

    pinMode(clima_vcc, OUTPUT);
    pinMode(umidade_solo_vcc, OUTPUT);
    pinMode(luz_vcc, OUTPUT);
    pinMode(chuva_vcc, OUTPUT);
    pinMode(aguar_vcc, OUTPUT);
    pinMode(aguar_ini, OUTPUT);

    dht.begin();
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
    if (MQTT.connected())
    {
        sub_mqtt();
        return;
    }
    while (!MQTT.connected())
    {
        Serial.println("!!! Conectando-se ao MQTT !!!");
        if (MQTT.connect(section_id))
        {
            Serial.print("Conectado a '");
            Serial.print(broker);
            Serial.println("'");
            sub_mqtt();
            return;
        }
    }
}

void get_data_mqtt(char *topic, byte *payload, unsigned int length)
{
    String mensagem;
    for (int i = 0; i < length; i++)
    {
        char c = (char)payload[i];
        mensagem += c;
    }
    Serial.println(mensagem);
}

void pub_mqtt()
{
    char json[255];
    sprintf(json, "{\"t\":%02.02f, \"u\":%02.02f, \"uS\":%02.02f, \"l\":%02.02f, \"c\":%02.02f}", _temperatura, _umidade, _umidadeSolo, _luz, _chuva);
    MQTT.publish(topico, json);
}

void sub_mqtt()
{
    MQTT.subscribe(topico);
}

void loop()
{
    conectarWiFi();
    conectarMQTT();

    temp_umid();
    umid_solo();
    qtd_chuva();
    qtd_luz();

    pub_mqtt();
    MQTT.loop();
    Serial.println("Fim do loop");
}

void temp_umid()
{
    digitalWrite(clima_vcc, HIGH);
    delay(tempo_l);
    _temperatura = dht.readTemperature();
    _umidade = dht.readHumidity();
    digitalWrite(clima_vcc, LOW);
}

void umid_solo()
{
    digitalWrite(umidade_solo_vcc, HIGH);
    delay(tempo_l);
    _umidadeSolo = analogRead(umidade_solo);
    digitalWrite(umidade_solo_vcc, LOW);
}

void qtd_chuva()
{
    digitalWrite(chuva_vcc, HIGH);
    delay(tempo_l);
    _chuva = analogRead(chuva);
    digitalWrite(chuva_vcc, LOW);
}

void qtd_luz()
{
    digitalWrite(luz_vcc, HIGH);
    delay(tempo_l);
    _luz = analogRead(luz);
    digitalWrite(luz_vcc, LOW);
}

void aguar_planta()
{
    digitalWrite(aguar_ini, LOW);
    delay(tempo_l);
    digitalWrite(aguar_ini, HIGH);
}
