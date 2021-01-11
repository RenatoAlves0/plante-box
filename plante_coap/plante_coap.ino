#include <DHT.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

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
#define url_sensores "sensores"
#define servidor IPAddress(192, 168, 0, 5)
#define porta 5683
#define ntpServer "pool.ntp.org"
#define gmtOffset_sec -3 * 3600
#define daylightOffset_sec -3 * 3600

WiFiUDP udp;
Coap coap(udp);
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
void put_coap();
void conectarWiFi();
void callback_response(CoapPacket &packet, IPAddress ip, int port);

void setup()
{
    Serial.begin(115200);
    conectarWiFi();

    coap.response(callback_response);
    coap.start();

    pinMode(clima_vcc, OUTPUT);
    pinMode(umidade_solo_vcc, OUTPUT);
    pinMode(luz_vcc, OUTPUT);
    pinMode(chuva_vcc, OUTPUT);
    pinMode(aguar_c, OUTPUT);
    digitalWrite(aguar_c, HIGH);

    dht.begin();

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
}

void printLocalTime()
{
    struct tm timeinfo;
    struct tm *time;
    time_t unixtime;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("!!! Erro ao obter tempo !!!");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    time = &timeinfo;
    unixtime = mktime(time);
    Serial.println("UnixTime");
    Serial.println(unixtime);

    timeval tv;
    tv.tv_sec = unixtime;
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

void callback_response(CoapPacket &packet, IPAddress ip, int port)
{
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = NULL;

    String message(p);
    // Serial.println("[Resposta do Servidor]");
    // Serial.println(message);

    if (message.equals("1"))
        regar = true;
    else if (message.equals("0"))
        regar = false;
}

void put_coap()
{
    struct timeval agora;
    gettimeofday(&agora, NULL);
    char json[1024];
    _umidadeSolo = _umidadeSolo / 40.95;
    _luz = _luz / 40.95;
    _chuva = _chuva / 40.95;
    sprintf(json, "{\"t\":%02.02f,\"u\":%02.02f,\"uS\":%02.02f,\"l\":%02.02f,\"c\":%02.02f, \"envio_s\":%u, \"envio_us\":%u}", _temperatura, _umidade, 100 - _umidadeSolo, _luz, 100 - _chuva, agora.tv_sec, agora.tv_usec);
    int msgid = coap.put(servidor, porta, url_sensores, json);
}

void loop()
{
    conectarWiFi();
    delay(1000);
    coap.loop();

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
        put_coap();
    }
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
    Serial.println("Ligando o Irrigador");
    digitalWrite(aguar_c, LOW);
    regando = true;
}

void finalizar_rega()
{
    acc_executa = 0;
    Serial.println("Desligando o Irrigador");
    digitalWrite(aguar_c, HIGH);
    regando = false;
}