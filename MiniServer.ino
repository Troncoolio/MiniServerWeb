/*
 * ESP32 Web Server - Control de LED vía WiFi
 * 
 * Este programa permite controlar un LED conectado al GPIO 4
 * a través de una página web servida por el ESP32.
 * 
 * Conexiones:
 * - LED (con resistencia de 220Ω) entre GPIO 4 y GND
 * - Cátodo del LED (pata corta) a GND
 * - Ánodo del LED (pata larga) a GPIO 4 a través de la resistencia
 */

#include <WiFi.h>
#include <WebServer.h>

// ===== CONFIGURACIÓN DE RED (MODIFICABLE) =====
const char* ssid = "Nickely";     // Cambia por tu SSID
const char* password = "PocH*chos.3"; // Cambia por tu contraseña

// ===== CONFIGURACIÓN DEL LED =====
const int ledPin = 4;  // GPIO4
bool ledEstado = LOW;  // Estado actual del LED

// Crear servidor web en puerto 80
WebServer server(80);

// ===== CÓDIGO HTML DE LA PÁGINA =====
const char paginaWeb[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <title>Control ESP32</title>
    <style>
        body {
            font-family: Arial, Helvetica, sans-serif;
            text-align: center;
            background-color: #f0f0f0;
            margin: 0;
            padding: 50px;
        }
        h1 {
            color: #333;
            font-size: 2.5em;
        }
        .container {
            background-color: white;
            border-radius: 15px;
            padding: 30px;
            max-width: 400px;
            margin: 0 auto;
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
        }
        .boton {
            display: inline-block;
            width: 150px;
            height: 60px;
            margin: 15px;
            font-size: 24px;
            font-weight: bold;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            transition: all 0.3s;
            box-shadow: 0 2px 5px rgba(0,0,0,0.2);
        }
        .boton:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.3);
        }
        .boton:active {
            transform: translateY(1px);
            box-shadow: 0 1px 3px rgba(0,0,0,0.2);
        }
        .btn-on {
            background-color: #4CAF50;
            color: white;
        }
        .btn-on:hover {
            background-color: #45a049;
        }
        .btn-off {
            background-color: #f44336;
            color: white;
        }
        .btn-off:hover {
            background-color: #da190b;
        }
        .info {
            margin-top: 25px;
            font-size: 18px;
            color: #666;
            background-color: #f8f8f8;
            padding: 15px;
            border-radius: 8px;
            word-break: break-all;
        }
        .estado {
            font-size: 20px;
            margin: 15px 0;
            padding: 10px;
            border-radius: 5px;
        }
        .estado-encendido {
            background-color: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .estado-apagado {
            background-color: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Control ESP32</h1>
        
        <div class="estado %ESTADO_CLASS%" id="estadoDiv">
            Estado: <span id="estadoTexto">%ESTADO_TEXTO%</span>
        </div>
        
        <button class="boton btn-on" onclick="encenderLED()">ENCENDER</button>
        <button class="boton btn-off" onclick="apagarLED()">APAGAR</button>
        
        <div class="info">
            <p><strong>IP del ESP32:</strong> %IP_ESP32%</p>
            <p><span id="ultimoComando">%ULTIMO_COMANDO%</span></p>
            <p><em>Monitor Serial para más información</em></p>
        </div>
    </div>
    
    <script>
        function encenderLED() {
            actualizarEstado('on');
            fetch('/on');
        }
        
        function apagarLED() {
            actualizarEstado('off');
            fetch('/off');
        }
        
        function actualizarEstado(comando) {
            var estadoDiv = document.getElementById('estadoDiv');
            var estadoTexto = document.getElementById('estadoTexto');
            var ultimoComando = document.getElementById('ultimoComando');
            
            if (comando === 'on') {
                estadoDiv.className = 'estado estado-encendido';
                estadoTexto.innerHTML = 'ENCENDIDO';
                ultimoComando.innerHTML = 'ENCENDER';
            } else {
                estadoDiv.className = 'estado estado-apagado';
                estadoTexto.innerHTML = 'APAGADO';
                ultimoComando.innerHTML = 'APAGAR';
            }
        }
    </script>
</body>
</html>
)rawliteral";

// ===== FUNCIONES MANEJADORAS =====
void handleRoot() {
    Serial.println("Página principal solicitada");
    
    String html = String(paginaWeb);
    
    // Reemplazar marcadores con valores actuales
    html.replace("%ESTADO_TEXTO%", ledEstado ? "ENCENDIDO" : "APAGADO");
    html.replace("%ESTADO_CLASS%", ledEstado ? "estado-encendido" : "estado-apagado");
    html.replace("%IP_ESP32%", WiFi.localIP().toString());
    
    server.send(200, "text/html", html);
}

void handleOn() {
    ledEstado = HIGH;
    digitalWrite(ledPin, ledEstado);
    
    Serial.println("Comando: ENCENDER LED");
    Serial.printf("LED GPIO %d = ON\n", ledPin);
    Serial.printf("Cliente: %s\n", server.client().remoteIP().toString().c_str());
    
    String html = String(paginaWeb);
    html.replace("%ESTADO_TEXTO%", "ENCENDIDO");
    html.replace("%ESTADO_CLASS%", "estado-encendido");
    html.replace("%IP_ESP32%", WiFi.localIP().toString());
    html.replace("%ULTIMO_COMANDO%", "ENCENDER");
    
    server.send(200, "text/html", html);
}

void handleOff() {
    ledEstado = LOW;
    digitalWrite(ledPin, ledEstado);
    
    Serial.println("Comando: APAGAR LED");
    Serial.printf("LED GPIO %d = OFF\n", ledPin);
    Serial.printf("Cliente: %s\n", server.client().remoteIP().toString().c_str());
    
    String html = String(paginaWeb);
    html.replace("%ESTADO_TEXTO%", "APAGADO");
    html.replace("%ESTADO_CLASS%", "estado-apagado");
    html.replace("%IP_ESP32%", WiFi.localIP().toString());
    html.replace("%ULTIMO_COMANDO%", "APAGAR");
    
    server.send(200, "text/html", html);
}

void handleNotFound() {
    server.send(404, "text/plain", "404: Página no encontrada");
}

// ===== SETUP =====
void setup() {
    // Inicializar monitor serial
    Serial.begin(115200);
    Serial.println("\n\n=== ESP32 WEB SERVER ===");
    Serial.println("Iniciando...");
    
    // Configurar pin del LED
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, ledEstado);
    Serial.printf("LED configurado en GPIO %d\n", ledPin);
    
    // Conectar a WiFi
    Serial.printf("Conectando a WiFi: %s\n", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 40) {
        delay(500);
        Serial.print(".");
        intentos++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi conectado!");
        Serial.printf("   Dirección IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("   MAC Address: %s\n", WiFi.macAddress().c_str());
    } else {
        Serial.println("\nError: No se pudo conectar a WiFi");
        Serial.println("   Verifica SSID y contraseña");
        Serial.println("   El ESP32 continuará intentando conectarse...");
    }
    
    // Configurar rutas del servidor
    server.on("/", handleRoot);
    server.on("/on", handleOn);
    server.on("/off", handleOff);
    server.onNotFound(handleNotFound);
    
    // Iniciar servidor
    server.begin();
    Serial.println("Servidor web iniciado en puerto 80");
    Serial.printf("ruta http://%s\n\n", WiFi.localIP().toString().c_str());
}

// ===== LOOP PRINCIPAL =====
void loop() {
    server.handleClient();
    
    // Verificar estado de WiFi cada 30 segundos
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 30000) {
        lastCheck = millis();
        
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi desconectado, reconectando...");
            WiFi.reconnect();
        }
    }
    
    delay(10);
}
