#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "FS.h"
#include "SPIFFS.h"

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
#define tempo 2500

#define ssid "..."
#define password "windows10mobile1"
#define topico_plantacao_principal "plante_plantacao_principal.5d699b7e0762797037d35801"
#define topico_sensores_c "plante_sensores_c.5d699b7e0762797037d35801"
#define topico_sensores "plante_sensores.5d699b7e0762797037d35801"
#define topico_regador_c "plante_regador_c.5d699b7e0762797037d35801"
#define topico_regador "plante_regador.5d699b7e0762797037d35801"
#define cliente_id "plante.5d699b7e0762797037d35801"
#define servidor "test.mosquitto.org"
#define porta 1883

//Variáveis globais
WiFiClient plante_box;
PubSubClient cliente(plante_box);
DHT dht(clima, DHT11);
float _temperatura = -100, _umidade = -100, _umidadeSolo = -100, _luz = -100, _chuva = -100;
int acc_executa = 0;
bool regar = false;

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
    if (!SPIFFS.begin(true))
        Serial.println("Erro ao montar SPIFFS");

    cliente.setServer(servidor, porta);
    cliente.setCallback(get_data_mqtt);
    conectarMQTT();

    pinMode(clima_vcc, OUTPUT);
    pinMode(umidade_solo_vcc, OUTPUT);
    pinMode(luz_vcc, OUTPUT);
    pinMode(chuva_vcc, OUTPUT);
    pinMode(aguar_vcc, OUTPUT);
    pinMode(aguar_ini, OUTPUT);

    dht.begin();
}

void ler(fs::FS &fs, const char *path)
{
    Serial.printf("Lendo de %s\r\n", path);
    File file = fs.open(path);
    if (!file || file.isDirectory())
    {
        Serial.printf("Falha o abrir %s\r\n", path);
        return;
    }

    Serial.printf("Resultado de %s\r\n", path);
    while (file.available())
        Serial.write(file.read());
    Serial.println();
}

void escrever(fs::FS &fs, const char *path, String message)
{
    Serial.printf("Escrevendo em %s\r\n", path);
    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.printf("Falha ao abrir %s\r\n", path);
        return;
    }
    if (file.print(message))
        Serial.printf("Gravado com sucesso em %s\r\n", path);
    else
        Serial.printf("Erro ao gravar em %s\r\n", path);
}

// void deletar(fs::FS &fs, const char *path)
// {
//     Serial.printf("Deletando %s\r\n", path);
//     if (fs.remove(path))
//         Serial.printf("%s\r\n deletado", path);
//     else
//         Serial.printf("Erro ao deletar %s\r\n", path);
// }

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
    Serial.println(mensagem);
    if (strcmp(topic, topico_plantacao_principal) == 0)
    {
        Serial.println(topic);
        escrever(SPIFFS, "/plantacao_principal.txt", mensagem);
        ler(SPIFFS, "/plantacao_principal.txt");
    }
}

void pub_mqtt()
{
    char json[255];
    _umidadeSolo = _umidadeSolo / 40.95;
    _luz = _luz / 40.95;
    _chuva = _chuva / 40.95;
    sprintf(json, "{\"t\":%02.02f,\"u\":%02.02f,\"uS\":%02.02f,\"l\":%02.02f,\"c\":%02.02f}", _temperatura, _umidade, _umidadeSolo, _luz, _chuva);
    cliente.publish(topico_sensores, json);
    cliente.publish(topico_sensores_c, json);
}

void sub_mqtt()
{
    cliente.subscribe(topico_regador);
    cliente.subscribe(topico_regador_c);
    cliente.subscribe(topico_plantacao_principal);
}

void loop()
{
    conectarWiFi();
    conectarMQTT();
    delay(tempo);
    cliente.loop();

    if (acc_executa == 24 / 4) //executa a cada 1 min
    {
        acc_executa = 0;
        delay(tempo_l);
        temp_umid();
        umid_solo();
        qtd_chuva();
        qtd_luz();
        pub_mqtt();
    }

    acc_executa = acc_executa + 1;
    Serial.println("...");
}

void temp_umid()
{
    digitalWrite(clima_vcc, HIGH);
    delay(tempo);
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
    digitalWrite(aguar_ini, HIGH);
}