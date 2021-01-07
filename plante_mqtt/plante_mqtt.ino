#include <DHT.h>
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

#define aguar_c 19

#define tempo_l 1500
#define tempo 2500

#define ssid "..."
#define password "windows10mobile1"
#define topico_sensores "sensores"
#define topico_regador "regador"
#define cliente_id "plante_box"
#define servidor IPAddress(192, 168, 0, 5)
#define porta 1883

WiFiClient plante_box;
PubSubClient cliente(plante_box);
DHT dht(clima, DHT11);
float _temperatura = -100, _umidade = -100, _umidadeSolo = -100, _luz = -100, _chuva = -100;
int acc_executa = 0;
bool regar = false, regando = false;

void umid_solo();
void temp_umid();
void qtd_chuva();
void qtd_luz();
void iniciar_rega();
void finalizar_rega();
void conectarWiFi();
void conectarMQTT();
void pub_mqtt();
void sub_mqtt();
void get_data_mqtt(char *topic, byte *payload, unsigned int length);

void setup()
{
    Serial.begin(115200);
    conectarWiFi();

    cliente.setServer(servidor, porta);
    cliente.setCallback(get_data_mqtt);
    conectarMQTT();

    pinMode(clima_vcc, OUTPUT);
    pinMode(umidade_solo_vcc, OUTPUT);
    pinMode(luz_vcc, OUTPUT);
    pinMode(chuva_vcc, OUTPUT);
    pinMode(aguar_c, OUTPUT);
    digitalWrite(aguar_c, HIGH);

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
    while (!cliente.connected())
    {
        Serial.println("!!! Conectando-se ao cliente !!!");
        if (cliente.connect(cliente_id))
        {
            Serial.print("Conectado a '");
            Serial.print(servidor);
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
    if (strcmp(topic, topico_regador) == 0)
    {
        if ((char)payload[0] == '1')
            regar = true;
        else
            regar = false;
    }
}

void pub_mqtt()
{
    char json[60];
    _umidadeSolo = _umidadeSolo / 40.95;
    _luz = _luz / 40.95;
    _chuva = _chuva / 40.95;
    sprintf(json, "{\"t\":%02.02f,\"u\":%02.02f,\"uS\":%02.02f,\"l\":%02.02f,\"c\":%02.02f}", _temperatura, _umidade, 100 - _umidadeSolo, _luz, 100 - _chuva);
    cliente.publish(topico_sensores, json);
}

void sub_mqtt()
{
    cliente.subscribe(topico_regador);
}

void loop()
{
    conectarWiFi();
    conectarMQTT();
    delay(tempo);
    cliente.loop();

    if (regar && !regando)
        iniciar_rega();
    if (!regar && regando)
        finalizar_rega();
    if (regando || acc_executa == 0)
    {
        delay(tempo_l);
        umid_solo();
        temp_umid();
        qtd_chuva();
        qtd_luz();
        pub_mqtt();
    }

    // acc_executa++;
    // if (acc_executa >= 6) //6 equivale a 15 segundos, 24 equivale a 1 minuto, 120 equivale a 5 minutos
    // acc_executa = 0;
    // Serial.println(acc_executa);
}

void temp_umid()
{
    digitalWrite(clima_vcc, HIGH);
    delay(tempo);
    if (dht.readTemperature() + 101 > 0)
        _temperatura = dht.readTemperature();
    if (dht.readTemperature() + 101 > 0)
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

void iniciar_rega()
{
    Serial.println("Ligando regador");
    digitalWrite(aguar_c, LOW);
    regando = true;
}

void finalizar_rega()
{
    acc_executa = 0;
    Serial.println("Desligando regador");
    digitalWrite(aguar_c, HIGH);
    regando = false;
}