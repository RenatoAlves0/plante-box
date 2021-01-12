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
#define cliente_id "plante_box"
#define servidor IPAddress(192, 168, 0, 5)
#define porta 1883
#define ntpServer "pool.ntp.org"
#define gmtOffset_sec -3 * 3600
#define daylightOffset_sec -3 * 3600

WiFiClient plante_box;
PubSubClient cliente(plante_box);

DHT dht(clima, DHT11);
float _temperatura = -100, _umidade = -100, _umidadeSolo = -100, _luz = -100, _chuva = -100;
int acc_executa = 0, id = 0;
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

void setup()
{
    Serial.begin(115200);
    conectarWiFi();

    cliente.setKeepAlive(18000); //5h
    cliente.setServer(servidor, porta);
    conectarMQTT();

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
    _umidadeSolo = _umidadeSolo / 40.95;
    _luz = _luz / 40.95;
    _chuva = _chuva / 40.95;
    // gettimeofday(&agora, NULL);

    char *topicos[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    for (int i = 0; i <= 9; i++)
    {
        delay(1000);
        gettimeofday(&agora, NULL);
        sprintf(json, "{\"id\":%d,\"t\":%02.02f,\"u\":%02.02f,\"uS\":%02.02f,\"l\":%02.02f,\"c\":%02.02f, \"envio_s\":%u, \"envio_us\":%u}", id, _temperatura, _umidade, 100 - _umidadeSolo, _luz, 100 - _chuva, agora.tv_sec, agora.tv_usec);
        cliente.publish(topicos[i], json);
    }

    // sprintf(json, "{\"id\":%d,\"t\":%02.02f,\"u\":%02.02f,\"uS\":%02.02f,\"l\":%02.02f,\"c\":%02.02f, \"envio_s\":%u, \"envio_us\":%u}", id, _temperatura, _umidade, 100 - _umidadeSolo, _luz, 100 - _chuva, agora.tv_sec, agora.tv_usec);
    // cliente.publish(topico_sensores, json);
    id = id + 1;
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