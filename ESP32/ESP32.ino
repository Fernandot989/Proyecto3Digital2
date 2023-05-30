// Universidad del Valle de Guatemala
// Electrónica Digital 2
// Oscar Donis (#21611) y Carlos Molina (#21253)
// 12/05/2023

/**************************** Configuration ************************************/

// UART
#include <Arduino.h>
#include <HardwareSerial.h>
#include <esp32-hal-uart.h>
#include <WiFi.h>
#include <WebServer.h>

/****************************** Variables ***********************************/
const int segmentPins[] = {12, 13, 33, 32, 14, 27, 26};

const byte digitPatterns[] = {
  0b00111111,   //0
  0b00000110,   //1
  0b01011011,   //2
  0b01001111,   //3
  0b01100110,   //4
  0b01101101,   //5
  0b01111101,   //6
  0b00000111,   //7
  0b01111111,   //8
  0b01101111    //9
};

const char* ssid = "F";  // SSID
const char* password = "donas989";  //Contraseña

WebServer server(80); // Object of WebServer(HTTP port, 80 is defult)
// UART 0
#define RXD0 3
#define TXD0 1

// UART 2
#define RXD2 16
#define TXD2 17

uint8_t paquete1;
uint8_t paquete2;

unsigned int parkinglot1status; 
unsigned int parkinglot2status;
unsigned int parkinglot3status; 
unsigned int parkinglot4status;
unsigned int parkinglot5status; 
unsigned int parkinglot6status;
unsigned int parkinglot7status; 
unsigned int parkinglot8status;

int parkinglotsavailable;

/************************ Prototipo de Funcion *******************************/
void displayNumber(int number);

/******************************* Main ****************************************/
void setup() {
  // ESP32 OUTPUT
  pinMode(2, OUTPUT);

  // DISPLAY OUTPUTS
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);

  
  // Configuramos los pines pertenecientes al módulo UART0
  pinMode(1, OUTPUT);
  pinMode(3, INPUT);

  // Configuramos los pines pertenecientes al módulo UART2
  pinMode(16, INPUT);
  pinMode(17, OUTPUT);

  // Iniciamos comunicación serial de UART0
  Serial1.begin(115200, SERIAL_8N1, RXD0, TXD0);

  // Iniciamos comunicación serial de UART2 (Baud Rate, 8 Bits|No Parity|1 Stop Bit, RX2 Pin, TX2 Pin)
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  WiFi.begin(ssid, password);
  server.on("/", handle_OnConnect); // Directamente desde e.g. 192.168.0.8
  server.onNotFound(handle_NotFound);
  server.begin();
  delay(100);
}

void loop() {  

  // Obtenemos dato del Segmento de Parqueo 1
  if (Serial1.available()){
    paquete1 = Serial1.read();
  }

  // Obtenemos dato del Segmento de Parqueo 2 
  if (Serial2.available()){
    paquete2 = Serial2.read();
  }
  
  // Actualizamos los estados de cada parqueo individual
  parkinglot1status = paquete1 & 0b00000001; 
  parkinglot2status = (paquete1 >> 1) & 0b00000001;
  parkinglot3status = (paquete1 >> 2) & 0b00000001;
  parkinglot4status = (paquete1 >> 3) & 0b00000001;

  parkinglot5status = paquete2 & 0b00000001; 
  parkinglot6status = (paquete2 >> 1) & 0b00000001;
  parkinglot7status = (paquete2 >> 2) & 0b00000001;
  parkinglot8status = (paquete2 >> 3) & 0b00000001;

  // Calibramos el número de parqueos disponibles 
  parkinglotsavailable = parkinglot1status + parkinglot2status + parkinglot3status + parkinglot4status+ parkinglot5status + parkinglot6status + parkinglot7status + parkinglot8status;

  // Mostramos el número de parqueos disponibles en el display de 7 segmentos
  displayNumber(parkinglotsavailable);

  // AQUÍ EMPIEZAS TU CON EL WEB SERVER
  server.handleClient(); 
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}
void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(parkinglot1status, parkinglot2status, parkinglot3status, parkinglot4status, parkinglot5status, parkinglot6status, parkinglot7status, parkinglot8status));
}

String SendHTML(uint8_t parking1, uint8_t parking2, uint8_t parking3, uint8_t parking4, uint8_t parking5, uint8_t parking6, uint8_t parking7, uint8_t parking8) {
  String ptr = "<!DOCTYPE html>";
  ptr += "<html>\n";
  ptr += "<head>";
  ptr += "<meta name=viewport content=width=device-width, initial-scale=1.0, user-scalable=no>\n";
  ptr += "<link rel=stylesheet href=https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css integrity=sha384-9aIt2nRpC12Uk9gS9baDl411NQApFmC26EwAOH8WgZl5MYYxFfc+NcPb1dKGj7Sk crossorigin=anonymous>\n";
  ptr += "<title>Parqueo</title>\n";
  ptr += "<script>";
  ptr += "  function autoRefresh() {";
  ptr += "    window.location = window.location.href;";
  ptr += "  }";
  ptr += "  setInterval('autoRefresh()', 2000);";
  ptr += "</script>";
  ptr += "<style>html{font-family:Helvetica;display:inline-block;margin:0 auto;text-align:center\n";
  ptr += "justify-content:center}body{margin:0;height:100vh;display:flex;flex-direction:column;justify-content:center;align-items:center;background:linear-gradient(#03cb06 33%,#ca0202)}h1{font-family:monospace;color:white;margin:30px auto 30px;padding:20px;border:2px solid black;border-radius:10px;background:black}h3{color:#444;margin-bottom:50px}p{font-size:14px;color:#888;margin-bottom:10px}table{width:500px;height:400px;border:3px solid black;border-radius:10px;text-align:center}.table-success th,.table-danger th{background-color:black;color:white;border:solid black}</style></head>";
  ptr += "<body>\n";
  ptr += "<h1>Estacionamiento</h1>\n";
  ptr += "<br>\n";
  ptr += "<table>\n";
  ptr += "<thead class=table-dark>\n";
  ptr += "<tr>\n";
  ptr += "<th scope=col># Parqueo</th>\n";
  ptr += "<th scope=col>Estado</th>\n";
  ptr += "</tr>\n";
  ptr += "</thead>\n";
  ptr += "<tbody>\n";
  if (parking1)
  {
    ptr += "<tr class=table-success>\n";
    ptr += "<th scope=row>1</th>\n";
    ptr += "<td> Disponible &#128664</td>\n";
    ptr += "</tr>\n";
  }
  else
  {
    ptr += "<tr class=table-danger>\n";
    ptr += "<th scope=row>1</th>\n";
    ptr += "<td>Ocupado &#128683</td>\n";
    ptr += "</tr>\n";
  }
  if (parking2)
  {
    ptr += "<tr class=table-success>\n";
    ptr += "<th scope=row>2</th>\n";
    ptr += "<td> Disponible &#128664</td>\n";
    ptr += "</tr>\n";
  }
  else
  {
    ptr += "<tr class=table-danger>\n";
    ptr += "<th scope=row>2</th>\n";
    ptr += "<td>Ocupado &#128683</td>\n";
    ptr += "</tr>\n";
  }
  if (parking3)
  {
    ptr += "<tr class=table-success>\n";
    ptr += "<th scope=row>3</th>\n";
    ptr += "<td> Disponible &#128664</td>\n";
    ptr += "</tr>\n";
  }
  else
  {
    ptr += "<tr class=table-danger>\n";
    ptr += "<th scope=row>3</th>\n";
    ptr += "<td>Ocupado &#128683</td>\n";
    ptr += "</tr>\n";
  }
  if (parking4)
  {
    ptr += "<tr class=table-success>\n";
    ptr += "<th scope=row>4</th>\n";
    ptr += "<td> Disponible &#128664</td>\n";
    ptr += "</tr>\n";
  }
  else
  {
    ptr += "<tr class=table-danger>\n";
    ptr += "<th scope=row>4</th>\n";
    ptr += "<td>Ocupado &#128683</td>\n";
    ptr += "</tr>\n";
  }
  if (parking5)
  {
    ptr += "<tr class=table-success>\n";
    ptr += "<th scope=row>5</th>\n";
    ptr += "<td> Disponible &#128664</td>\n";
    ptr += "</tr>\n";
  }
  else
  {
    ptr += "<tr class=table-danger>\n";
    ptr += "<th scope=row>5</th>\n";
    ptr += "<td>Ocupado &#128683</td>\n";
    ptr += "</tr>\n";
  }
  if (parking6)
  {
    ptr += "<tr class=table-success>\n";
    ptr += "<th scope=row>6</th>\n";
    ptr += "<td> Disponible &#128664</td>\n";
    ptr += "</tr>\n";
  }
  else
  {
    ptr += "<tr class=table-danger>\n";
    ptr += "<th scope=row>6</th>\n";
    ptr += "<td>Ocupado &#128683</td>\n";
    ptr += "</tr>\n";
  }
  if (parking7)
  {
    ptr += "<tr class=table-success>\n";
    ptr += "<th scope=row>7</th>\n";
    ptr += "<td> Disponible &#128664</td>\n";
    ptr += "</tr>\n";
  }
  else
  {
    ptr += "<tr class=table-danger>\n";
    ptr += "<th scope=row>7</th>\n";
    ptr += "<td>Ocupado &#128683</td>\n";
    ptr += "</tr>\n";
  }
  if (parking8)
  {
    ptr += "<tr class=table-success>\n";
    ptr += "<th scope=row>8</th>\n";
    ptr += "<td> Disponible &#128664</td>\n";
    ptr += "</tr>\n";
  }
  else
  {
    ptr += "<tr class=table-danger>\n";
    ptr += "<th scope=row>8</th>\n";
    ptr += "<td>Ocupado &#128683</td>\n";
    ptr += "</tr>\n";
  }
  ptr += "</tbody>\n";
  ptr += "</table>\n";
  ptr += "<script src=https://code.jquery.com/jquery-3.5.1.slim.min.js integrity=sha384-DfXdz2htPH0lsSSs5nCTpuj/zy4C+OGpamoFVy38MVBnE+IbbVYUew+OrCXaRkfj crossorigin=anonymous></script>\n";
  ptr += "<script src=https://cdn.jsdelivr.net/npm/popper.js@1.16.0/dist/umd/popper.min.js integrity=sha384-Q6E9RHvbIyZFJoft+2mJbHaEWldlvI9IOYy5n3zV9zzTtmI3UksdQRVvoxMfooAo crossorigin=anonymous></script>\n";
  ptr += "<script src=https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/js/bootstrap.min.js integrity=sha384-OgVRvuATP1z7JjHLkuOU7Xw704+h835Lr+6QL9UvYjZE3Ipu6Tp75j7Bh/kR0JKI crossorigin=anonymous></script>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
void displayNumber(int number) {
  if (number >= 0 && number <= 9) {
    byte segments = digitPatterns[number];
    for (int i = 0; i < 7; i++) {
      digitalWrite(segmentPins[i], bitRead(segments, i));
    }
  }
}