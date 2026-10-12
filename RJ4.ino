#define ENABLE_USER_AUTH
#define ENABLE_DATABASE


#include <Arduino.h>

#include <WiFi.h>

#include <WiFiClientSecure.h>

#include <FirebaseClient.h>

#include <LittleFS.h>

#include <ESPAsyncWebServer.h>

#include <AsyncTCP.h>

#include <DHT.h>

#include <time.h>


/* =====================================================
   DHT11
===================================================== */

#define DHTPIN 4

#define DHTTYPE DHT11


DHT dht(
    DHTPIN,
    DHTTYPE
);


/* =====================================================
   FIREBASE
===================================================== */

#define API_KEY \
    "AIzaSyABolysCm-05TJ1gOL4EYXXx2130TUOaq0"


#define DATABASE_URL \
    "https://rj-website-29f84-default-rtdb.europe-west1.firebasedatabase.app"


#define USER_EMAIL \
    "bebosorj0@gmail.com"


#define USER_PASSWORD \
    "beboso123"


/* =====================================================
   FIREBASE OBJECTS
===================================================== */

UserAuth user_auth(

    API_KEY,

    USER_EMAIL,

    USER_PASSWORD,

    3000
);


FirebaseApp app;


WiFiClientSecure ssl_client;


AsyncClientClass aClient(
    ssl_client
);


RealtimeDatabase Database;


/* =====================================================
   WEB SERVER
===================================================== */

AsyncWebServer server(80);


/* =====================================================
   WIFI MANAGER
===================================================== */

const char *AP_SSID =
    "ESP-WIFI-MANAGER";


bool wifiManagerMode =
    false;


/* =====================================================
   SENSOR TIMER
===================================================== */

unsigned long lastSensorRead =
    0;


const unsigned long SENSOR_INTERVAL =
    10000;


/* =====================================================
   FUNCTION DECLARATIONS
===================================================== */

void processFirebase(
    AsyncResult &aResult
);


bool connectToSavedWiFi();


void startWiFiManager();


void startMainWebServer();


void setupFirebase();


void sendSensorData();


String readFile(
    const char *path
);


bool writeFile(
    const char *path,
    const String &data
);


String getDateString();


String getTimeString();


/* =====================================================
   WIFI MANAGER HTML
===================================================== */

const char WIFI_MANAGER_HTML[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html lang="en">

<head>

<meta charset="UTF-8">

<meta
    name="viewport"
    content="width=device-width, initial-scale=1.0"
>

<title>
RJ | ESP WiFi Manager
</title>


<style>

* {

    margin: 0;

    padding: 0;

    box-sizing: border-box;

    font-family:
        Arial,
        Helvetica,
        sans-serif;
}


body {

    min-height: 100vh;

    display: flex;

    justify-content: center;

    align-items: center;

    padding: 20px;

    background:
        linear-gradient(
            135deg,
            #0f172a,
            #111827,
            #1e293b
        );

    color: #e5e7eb;
}


.wifi-card {

    width: 100%;

    max-width: 500px;

    padding: 35px;

    background:
        linear-gradient(
            145deg,
            #1e293b,
            #172033
        );

    border-radius: 20px;

    border:
        1px solid
        #334155;

    box-shadow:
        0 15px 40px
        rgba(0,0,0,.45);
}


.wifi-icon {

    text-align: center;

    font-size: 42px;

    margin-bottom: 10px;
}


.wifi-card h1 {

    text-align: center;

    color: #38bdf8;

    margin-bottom: 10px;

    font-size: 28px;
}


.wifi-card p {

    text-align: center;

    color: #94a3b8;

    margin-bottom: 28px;

    font-size: 14px;
}


.form-group {

    margin-bottom: 18px;
}


.form-group label {

    display: block;

    color: #7dd3fc;

    font-weight: bold;

    margin-bottom: 7px;

    font-size: 14px;
}


.form-group input {

    width: 100%;

    padding: 13px 15px;

    border:
        1px solid
        #475569;

    border-radius: 8px;

    outline: none;

    font-size: 16px;

    color: #f8fafc;

    background: #0f172a;
}


.form-group input::placeholder {

    color: #64748b;
}


.form-group input:focus {

    border-color:
        #3b82f6;

    box-shadow:
        0 0 0 3px
        rgba(59,130,246,.15);
}


.wifi-button {

    width: 100%;

    margin-top: 10px;

    padding: 13px 24px;

    border: none;

    border-radius: 8px;

    background:
        linear-gradient(
            135deg,
            #2563eb,
            #1d4ed8
        );

    color: white;

    font-size: 15px;

    cursor: pointer;

    font-weight: bold;
}


.info {

    margin-top: 20px;

    padding: 15px;

    border-radius: 12px;

    background: #172033;

    border:
        1px solid
        #334155;

    color: #94a3b8;

    font-size: 14px;

    line-height: 1.6;
}


.info strong {

    color: #7dd3fc;
}

</style>

</head>


<body>


<div class="wifi-card">


<div class="wifi-icon">
📶
</div>


<h1>
RJ WiFi Manager
</h1>


<p>
Configure the WiFi connection
for your ESP32.
</p>


<form
    action="/"
    method="POST"
>


<div class="form-group">

<label for="ssid">
WiFi SSID
</label>

<input
    type="text"
    id="ssid"
    name="ssid"
    placeholder="Enter WiFi name"
    required
>

</div>


<div class="form-group">

<label for="pass">
WiFi Password
</label>

<input
    type="password"
    id="pass"
    name="pass"
    placeholder="Enter WiFi password"
    required
>

</div>


<div class="form-group">

<label for="ip">
IP Address
</label>

<input
    type="text"
    id="ip"
    name="ip"
    placeholder="Optional - leave blank for DHCP"
>

</div>


<div class="form-group">

<label for="gateway">
Gateway Address
</label>

<input
    type="text"
    id="gateway"
    name="gateway"
    placeholder="Optional - leave blank for DHCP"
>

</div>


<button
    type="submit"
    class="wifi-button"
>
Save
</button>


</form>


<div class="info">

<strong>
WiFi Setup
</strong>

<br><br>

1. Connect to
<strong>ESP-WIFI-MANAGER</strong>.

<br>

2. Open
<strong>192.168.4.1</strong>.

<br>

3. Enter your WiFi information.

<br>

4. Click Save.

<br>

5. ESP32 will restart.

<br>

6. Open the ESP32 IP address.

</div>


</div>


</body>

</html>

)rawliteral";


/* =====================================================
   READ FILE
===================================================== */

String readFile(
    const char *path
)
{

    if (
        !LittleFS.exists(path)
    )
    {

        return "";
    }


    File file =
        LittleFS.open(
            path,
            "r"
        );


    if (!file)
    {

        return "";
    }


    String data =
        file.readString();


    file.close();


    data.trim();


    return data;
}


/* =====================================================
   WRITE FILE
===================================================== */

bool writeFile(
    const char *path,
    const String &data
)
{

    File file =
        LittleFS.open(
            path,
            "w"
        );


    if (!file)
    {

        Serial.print(
            "Failed to open file: "
        );

        Serial.println(
            path
        );

        return false;
    }


    file.print(data);


    file.close();


    return true;
}


/* =====================================================
   CONNECT SAVED WIFI
===================================================== */

bool connectToSavedWiFi()
{

    String ssid =
        readFile(
            "/ssid.txt"
        );


    String pass =
        readFile(
            "/pass.txt"
        );


    String ip =
        readFile(
            "/ip.txt"
        );


    String gateway =
        readFile(
            "/gateway.txt"
        );


    if (
        ssid.length() == 0
    )
    {

        Serial.println();

        Serial.println(
            "NO SAVED WIFI"
        );

        return false;
    }


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "       SAVED WIFI FOUND"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "SSID: "
    );

    Serial.println(
        ssid
    );


    WiFi.mode(
        WIFI_STA
    );


    delay(500);


    /* =========================================
       OPTIONAL STATIC IP
    ========================================= */

    if (
        ip.length() > 0 &&
        gateway.length() > 0
    )
    {

        IPAddress local_IP;

        IPAddress gateway_IP;


        if (
            local_IP.fromString(ip) &&
            gateway_IP.fromString(gateway)
        )
        {

            IPAddress subnet(
                255,
                255,
                255,
                0
            );


            if (
                WiFi.config(
                    local_IP,
                    gateway_IP,
                    subnet
                )
            )
            {

                Serial.println(
                    "Static IP configured."
                );

            }
            else
            {

                Serial.println(
                    "Static IP configuration failed."
                );
            }
        }
    }


    WiFi.begin(
        ssid.c_str(),
        pass.c_str()
    );


    Serial.print(
        "Connecting to WiFi"
    );


    unsigned long startTime =
        millis();


    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 20000
    )
    {

        Serial.print(
            "."
        );


        delay(500);
    }


    Serial.println();


    if (
        WiFi.status() ==
        WL_CONNECTED
    )
    {

        Serial.println();

        Serial.println(
            "================================="
        );

        Serial.println(
            "       WIFI CONNECTED"
        );

        Serial.println(
            "================================="
        );


        Serial.print(
            "SSID: "
        );

        Serial.println(
            WiFi.SSID()
        );


        Serial.print(
            "IP Address: "
        );

        Serial.println(
            WiFi.localIP()
        );


        Serial.print(
            "Gateway: "
        );

        Serial.println(
            WiFi.gatewayIP()
        );


        Serial.println();

        return true;
    }


    Serial.println(
        "Failed to connect to saved WiFi."
    );


    WiFi.disconnect(
        true
    );


    delay(1000);


    return false;
}


/* =====================================================
   START WIFI MANAGER
===================================================== */

void startWiFiManager()
{

    wifiManagerMode =
        true;


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "       WIFI MANAGER MODE"
    );

    Serial.println(
        "================================="
    );


    WiFi.mode(
        WIFI_AP
    );


    delay(500);


    bool apStarted =
        WiFi.softAP(
            AP_SSID
        );


    if (!apStarted)
    {

        Serial.println(
            "ERROR: Failed to start WiFi Manager AP!"
        );
    }


    delay(1000);


    Serial.print(
        "AP SSID: "
    );

    Serial.println(
        AP_SSID
    );


    Serial.print(
        "AP IP Address: "
    );

    Serial.println(
        WiFi.softAPIP()
    );


    /* =========================================
       WIFI MANAGER PAGE
    ========================================= */

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {

            request->send(
                200,
                "text/html",
                WIFI_MANAGER_HTML
            );

        }
    );


    /* =========================================
       SAVE WIFI
    ========================================= */

    server.on(
        "/",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {

            String ssid = "";

            String pass = "";

            String ip = "";

            String gateway = "";


            if (
                request->hasParam(
                    "ssid",
                    true
                )
            )
            {

                ssid =
                    request
                    ->getParam(
                        "ssid",
                        true
                    )
                    ->value();
            }


            if (
                request->hasParam(
                    "pass",
                    true
                )
            )
            {

                pass =
                    request
                    ->getParam(
                        "pass",
                        true
                    )
                    ->value();
            }


            if (
                request->hasParam(
                    "ip",
                    true
                )
            )
            {

                ip =
                    request
                    ->getParam(
                        "ip",
                        true
                    )
                    ->value();
            }


            if (
                request->hasParam(
                    "gateway",
                    true
                )
            )
            {

                gateway =
                    request
                    ->getParam(
                        "gateway",
                        true
                    )
                    ->value();
            }


            ssid.trim();

            pass.trim();

            ip.trim();

            gateway.trim();


            if (
                ssid.length() == 0 ||
                pass.length() == 0
            )
            {

                request->send(
                    400,
                    "text/plain",
                    "SSID and password are required."
                );

                return;
            }


            writeFile(
                "/ssid.txt",
                ssid
            );


            writeFile(
                "/pass.txt",
                pass
            );


            writeFile(
                "/ip.txt",
                ip
            );


            writeFile(
                "/gateway.txt",
                gateway
            );


            request->send(
                200,
                "text/html",

                "<html>"
                "<head>"
                "<meta name='viewport' "
                "content='width=device-width, initial-scale=1'>"
                "</head>"

                "<body style='font-family:Arial;"
                "text-align:center;"
                "padding:50px;"
                "background:#0f172a;"
                "color:#e5e7eb;'>"

                "<div style='background:#1e293b;"
                "padding:30px;"
                "border-radius:20px;"
                "max-width:500px;"
                "margin:auto;"
                "border:1px solid #334155;'>"

                "<h1 style='color:#38bdf8;'>"
                "WiFi Saved!"
                "</h1>"

                "<p>"
                "The ESP32 will restart and "
                "connect to the saved WiFi."
                "</p>"

                "<p>"
                "Please wait..."
                "</p>"

                "</div>"
                "</body>"
                "</html>"
            );


            delay(1500);


            ESP.restart();
        }
    );


    server.begin();


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        " WIFI MANAGER READY"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Connect to WiFi: "
    );

    Serial.println(
        AP_SSID
    );


    Serial.print(
        "Then open: http://"
    );


    Serial.println(
        WiFi.softAPIP()
    );


    Serial.println();
}


/* =====================================================
   MAIN WEB SERVER
===================================================== */

void startMainWebServer()
{

    wifiManagerMode =
        false;


    /* =========================================
       MAIN WEBSITE
    ========================================= */

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {

            if (
                LittleFS.exists(
                    "/index.html"
                )
            )
            {

                request->send(
                    LittleFS,
                    "/index.html",
                    "text/html"
                );

            }
            else
            {

                request->send(
                    404,
                    "text/plain",
                    "index.html not found."
                );
            }

        }
    );


    /* =========================================
       STATIC FILES
    ========================================= */

    server.serveStatic(
        "/",
        LittleFS,
        "/"
    );


    server.begin();


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "      MAIN WEB SERVER READY"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Website: http://"
    );


    Serial.println(
        WiFi.localIP()
    );


    Serial.println();
}


/* =====================================================
   FIREBASE CALLBACK
===================================================== */

void processFirebase(
    AsyncResult &aResult
)
{

    if (
        !aResult.isResult()
    )
    {

        return;
    }


    /* =========================================
       EVENT
    ========================================= */

    if (
        aResult.isEvent()
    )
    {

        Firebase.printf(

            "Firebase Event - task: %s, msg: %s, code: %d\n",

            aResult.uid().c_str(),

            aResult.eventLog()
                .message()
                .c_str(),

            aResult.eventLog()
                .code()
        );
    }


    /* =========================================
       DEBUG
    ========================================= */

    if (
        aResult.isDebug()
    )
    {

        Firebase.printf(

            "Firebase Debug - task: %s, msg: %s\n",

            aResult.uid().c_str(),

            aResult.debug().c_str()
        );
    }


    /* =========================================
       ERROR
    ========================================= */

    if (
        aResult.isError()
    )
    {

        Firebase.printf(

            "Firebase Error - task: %s, msg: %s, code: %d\n",

            aResult.uid().c_str(),

            aResult.error()
                .message()
                .c_str(),

            aResult.error()
                .code()
        );
    }


    /* =========================================
       PAYLOAD
    ========================================= */

    if (
        aResult.available()
    )
    {

        Firebase.printf(

            "Firebase Payload - task: %s, payload: %s\n",

            aResult.uid().c_str(),

            aResult.c_str()
        );
    }
}


/* =====================================================
   FIREBASE SETUP
===================================================== */

void setupFirebase()
{

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "       FIREBASE SETUP"
    );

    Serial.println(
        "================================="
    );


    ssl_client.setInsecure();


    Serial.println(
        "Initializing Firebase..."
    );


    initializeApp(

        aClient,

        app,

        getAuth(user_auth),

        processFirebase,

        "authTask"
    );


    app.getApp<RealtimeDatabase>(
        Database
    );


    Database.url(
        DATABASE_URL
    );


    Serial.println(
        "Firebase initialization started."
    );


    Serial.println();
}


/* =====================================================
   GET DATE
===================================================== */

String getDateString()
{

    struct tm timeinfo;


    if (
        !getLocalTime(
            &timeinfo
        )
    )
    {

        return "1970-01-01";
    }


    char buffer[20];


    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d",
        &timeinfo
    );


    return String(
        buffer
    );
}


/* =====================================================
   GET TIME
===================================================== */

String getTimeString()
{

    struct tm timeinfo;


    if (
        !getLocalTime(
            &timeinfo
        )
    )
    {

        return "00:00:00";
    }


    char buffer[20];


    strftime(
        buffer,
        sizeof(buffer),
        "%H:%M:%S",
        &timeinfo
    );


    return String(
        buffer
    );
}


/* =====================================================
   SEND SENSOR DATA
===================================================== */

void sendSensorData()
{

    /* =========================================
       CHECK FIREBASE
    ========================================= */

    if (
        !app.ready()
    )
    {

        Serial.println();

        Serial.println(
            "Firebase not ready yet..."
        );

        return;
    }


    /* =========================================
       READ DHT11
    ========================================= */

    float humidity =
        dht.readHumidity();


    float temperature =
        dht.readTemperature();


    /* =========================================
       CHECK SENSOR
    ========================================= */

    if (
        isnan(humidity) ||
        isnan(temperature)
    )
    {

        Serial.println();

        Serial.println(
            "================================="
        );

        Serial.println(
            "ERROR: Failed to read DHT11"
        );

        Serial.println(
            "================================="
        );

        return;
    }


    /* =========================================
       DATE AND TIME
    ========================================= */

    String date =
        getDateString();


    String time =
        getTimeString();


    /* =========================================
       BASE PATH
    ========================================= */

    String basePath =

        "/ESP32_Data/" +

        date +

        "/" +

        time;


    /* =========================================
       SENSOR PATHS
    ========================================= */

    String temperaturePath =

        basePath +

        "/temperature";


    String humidityPath =

        basePath +

        "/humidity";


    /* =========================================
       SERIAL MONITOR
    ========================================= */

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "       DHT11 SENSOR READING"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Temperature: "
    );

    Serial.print(
        temperature,
        1
    );

    Serial.println(
        " °C"
    );


    Serial.print(
        "Humidity: "
    );

    Serial.print(
        humidity,
        1
    );

    Serial.println(
        " %"
    );


    Serial.print(
        "Date: "
    );

    Serial.println(
        date
    );


    Serial.print(
        "Time: "
    );

    Serial.println(
        time
    );


    Serial.print(
        "Firebase base path: "
    );

    Serial.println(
        basePath
    );


    Serial.println();


    /* =========================================
       FIREBASE WRITE
    ========================================= */

    Database.set<float>(

        aClient,

        temperaturePath,

        temperature,

        processFirebase,

        "temperatureTask"
    );


    Database.set<float>(

        aClient,

        humidityPath,

        humidity,

        processFirebase,

        "humidityTask"
    );


    Serial.println(
        "Temperature write task sent."
    );


    Serial.println(
        "Humidity write task sent."
    );


    Serial.println();

    Serial.println(
        "================================="
    );
}


/* =====================================================
   SETUP
===================================================== */

void setup()
{

    Serial.begin(
        115200
    );


    delay(1000);


    Serial.println();

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "   RJ ACTIVITY 4"
    );

    Serial.println(
        "   ESP32 DHT11 FIREBASE MONITOR"
    );

    Serial.println(
        "================================="
    );


    /* =========================================
       LITTLEFS
    ========================================= */

    Serial.println();

    Serial.println(
        "Starting LittleFS..."
    );


    if (
        !LittleFS.begin(true)
    )
    {

        Serial.println(
            "LittleFS mount failed!"
        );


        while (true)
        {

            delay(1000);
        }
    }


    Serial.println(
        "LittleFS ready."
    );


    /* =========================================
       DHT11
    ========================================= */

    dht.begin();


    Serial.println(
        "DHT11 initialized."
    );


    /* =========================================
       WIFI
    ========================================= */

    bool connected =
        connectToSavedWiFi();


    if (!connected)
    {

        startWiFiManager();

        return;
    }


    /* =========================================
       NTP
    ========================================= */

    Serial.println(
        "Starting NTP time..."
    );


    /*
       Philippines
       UTC + 8
    */

    configTime(

        8 * 3600,

        0,

        "pool.ntp.org",

        "time.nist.gov",

        "time.google.com"
    );


    Serial.print(
        "Waiting for time"
    );


    struct tm timeinfo;


    int retry = 0;


    while (

        !getLocalTime(
            &timeinfo
        )

        &&

        retry < 20

    )
    {

        Serial.print(
            "."
        );


        delay(500);


        retry++;
    }


    Serial.println();


    if (
        getLocalTime(
            &timeinfo
        )
    )
    {

        Serial.println(
            "Time synchronized."
        );


        Serial.print(
            "Date: "
        );


        Serial.println(
            getDateString()
        );


        Serial.print(
            "Time: "
        );


        Serial.println(
            getTimeString()
        );

    }
    else
    {

        Serial.println(
            "WARNING: Time synchronization failed."
        );
    }


    /* =========================================
       FIREBASE
    ========================================= */

    setupFirebase();


    /* =========================================
       WEB SERVER
    ========================================= */

    startMainWebServer();


    /* =========================================
       READY
    ========================================= */

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "         SYSTEM READY"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Website: http://"
    );


    Serial.println(
        WiFi.localIP()
    );


    Serial.println();
}


/* =====================================================
   LOOP
===================================================== */

void loop()
{

    /* =========================================
       FIREBASE ASYNC TASKS
    ========================================= */

    if (
        !wifiManagerMode
    )
    {

        app.loop();
    }


    /* =========================================
       DHT11 EVERY 10 SECONDS
    ========================================= */

    if (

        !wifiManagerMode &&

        millis() -
        lastSensorRead >=
        SENSOR_INTERVAL

    )
    {

        lastSensorRead =
            millis();


        sendSensorData();
    }


    delay(10);
}