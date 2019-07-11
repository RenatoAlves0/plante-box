#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define clima 23
#define clima_vcc 22

#define luz 36
#define luz_vcc 32

#define solo 39
#define solo_vcc 25

#define chuva 34
#define chuva_vcc 27

#define aguar_ini 19
#define aguar_vcc 18

#define tempo_l 1500
#define tempo 6000

#define ssid "..."
#define password "windows10mobile1"
#define topico_dado_teste "plante_iot_dado_teste(renalves.oli)"
#define topico_dado_teste_novo "plante_iot_dado_teste_novo(renalves.oli)"
#define section_id "plante_iot_id(renalves.oli)"
#define broker "iot.eclipse.org"
#define broker_port 1883

//Variáveis globais
WiFiClient plante_iot;
PubSubClient MQTT(plante_iot);
DHT dht(clima, DHT11);

//Prototypes
void conectarWiFi();
void conectarMQTT();
void sub_mqtt(char *topic, byte *payload, unsigned int length);

void setup()
{
    Serial.begin(115200);
    conectarWiFi();

    MQTT.setServer(broker, broker_port);
    MQTT.setCallback(sub_mqtt);
    conectarMQTT();

    pinMode(clima_vcc, OUTPUT);
    pinMode(solo_vcc, OUTPUT);
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
        Serial.print("Conectando-se ao Wi-Fi ");
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
    if (!MQTT.connected())
    {
        Serial.print("Conectando-se ao MQTT ");
        while (!MQTT.connected())
        {
            MQTT.connect(section_id);
            Serial.print(".");
            delay(500);
        }
        Serial.println();
        Serial.print("Conectado a '");
        Serial.print(broker);
        Serial.println("'");
        MQTT.subscribe(topico_dado_teste);
        MQTT.subscribe(topico_dado_teste_novo);
    }
}

void sub_mqtt(char *topic, byte *payload, unsigned int length)
{
    String mensagem;
    for (int i = 0; i < length; i++)
    {
        char c = (char)payload[i];
        mensagem += c;
    }
    Serial.print("Mensagem recebida: ");
    Serial.println(mensagem);
}

void pub_mqtt()
{
    //   sprintf(json,  "{\"%s\":{\"value\":%02.02f, \"context\":{\"temperature\":%02.02f, \"humidity\":%02.02f}}}", VARIABLE_LABEL_TEMPERATURE, temperature, temperature, humidity);  
}

void loop()
{
    conectarMQTT();
    conectarWiFi();

    MQTT.publish(topico_dado_teste, "Dado_teste");
    MQTT.publish(topico_dado_teste_novo, "Dado_teste_novo");

    MQTT.loop();
    delay(5000);
    // aguar_planta();
    // delay(tempo);
    // temp_umid();
    // delay(tempo);
    // umid_solo();
    // delay(tempo);
    // qtd_chuva();
    // delay(tempo);
    // qtd_luz();
    // delay(tempo);
}

void temp_umid()
{
    digitalWrite(clima_vcc, HIGH);
    delay(tempo_l);
    double u = dht.readHumidity();
    double t = dht.readTemperature();
    digitalWrite(clima_vcc, LOW);
    Serial.print("Umidade: ");
    Serial.print(u);
    Serial.println(" %");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println(" *C");
    Serial.println();
}

void umid_solo()
{
    digitalWrite(solo_vcc, HIGH);
    delay(tempo_l);
    Serial.print("Umidade do solo: ");
    Serial.println(analogRead(solo));
    digitalWrite(solo_vcc, LOW);
    Serial.println();
}

void qtd_chuva()
{
    digitalWrite(chuva_vcc, HIGH);
    delay(tempo_l);
    Serial.print("Quantidade de chuva: ");
    Serial.println(analogRead(chuva));
    digitalWrite(chuva_vcc, LOW);
    Serial.println();
}

void qtd_luz()
{
    digitalWrite(luz_vcc, HIGH);
    delay(tempo_l);
    Serial.print("Quantidade de luz: ");
    Serial.println(analogRead(luz));
    digitalWrite(luz_vcc, LOW);
    Serial.println();
}

void aguar_planta()
{
    // digitalWrite(aguar_vcc, HIGH);
    delay(tempo_l);
    digitalWrite(aguar_ini, LOW);
    delay(tempo_l);
    digitalWrite(aguar_ini, HIGH);
    // digitalWrite(aguar_vcc, LOW);
    // Serial.println();
}
