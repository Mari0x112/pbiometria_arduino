#include <Adafruit_TinyUSB.h>
#include <bluefruit.h>

#define PIN_Vgas 5 // PIN gas
#define PIN_Vref 28 // PIN referencia gas
#define PIN_Vtemp 29 // PIN temperatura

int cont = 0;

double Vgas; // Valor de gas
double Vref; // Valor de referencia para el gas
double Vtemp; // Valor de la temperatura

// UUID del beacon, 16 carácteres como máximo
const uint8_t beaconUUID[16] = {'E', 'P', 'S', 'G', '-', 'G', 'T', 'I', '-', 'P', 'B', 'I', 'O', 'M', 'E', 'T'};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                            setup()
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void setup() {

  // Inicio Serial
  Serial.begin(115200);

  // Ajusto modo de los pines definidos
  pinMode(PIN_Vgas, INPUT);
  pinMode(PIN_Vref, INPUT);
  pinMode(PIN_Vtemp, INPUT);

  // Inicio modulo Bluefruit
  Bluefruit.begin();

  // Añado el nombre del beacon del dispositivo
  Bluefruit.setName("GTI-3A-Laura");
  Bluefruit.ScanResponse.addName();

  // Empiezo a retransmitir
  startAdvertising();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                            loop()
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void loop() {
  startAdvertising();
  cont++;

  delay(1000);

  Serial.print( " ** loop cont=" );
  Serial.println( cont );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//      Devuelve la lectura de gas O3 en el instante que se 
//      llama a la función
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                    leerGas() -> double
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double leerGas(){
  Vgas = analogRead(PIN_Vgas)/pow(2, 12)*3.3;
  Vref = analogRead(PIN_Vref)/pow(2, 12)*3.3;

  double LecturaFinal = Vgas-Vref;
  if(LecturaFinal<0) LecturaFinal = LecturaFinal*(-1);

  return LecturaFinal;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        Devuelve la lectura de Temperatura en ºC en el  
//        instante que se llama a la función
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                 leerTemperatura() -> double
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double leerTemperatura(){
  Vtemp = analogRead(PIN_Vtemp)/pow(2, 12)*3.3;
  double Temp = 87*Vtemp-18;
  return Temp;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        Calcula la concentración de gas O3 en PPM
//        a partir de su la lectura instantánea 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                  calcularPPM() -> double
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double calcularPPM(){
  double M = 56.91 * 499 * pow(10, (-6));
  double resultado = leerGas()/M;
  return resultado;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//        Corrige el calculo de PPM en función de la
//        temperatura instantánea obtenida
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//               ppmFinal() -> double
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double ppmFinal(){
  double incrementoT = leerTemperatura() - 20;
  double corregida = incrementoT * 0.003;
  double resultado = calcularPPM() - corregida;
  return resultado;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//           Configuración y inicialización de las
//           publicaciones beacon BLE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                       startAdvertising()
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void startAdvertising() {

  // Detengo las publicaciones para editar la configuración
  Bluefruit.Advertising.stop(); 

  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Añadir nombre al beacon
  Bluefruit.Advertising.addName();

  // Definoun objeto BLEBeacon con el UUID anterior, major, minor y rssi
  // Dentro de major y minor publicaremos los valores de temperatura y ppm de O3
  BLEBeacon elBeacon( beaconUUID, ppmFinal()*100, leerTemperatura()*100, 73 );
  Serial.println(calcularPPM());
  Serial.println(correccionTemp());

  // Le asignamos un fabricante, apple para iBeacons
  elBeacon.setManufacturer( 0x004c ); // ID de apple

  // Establecemos el objeto definido como Beacon a retransmitir
  Bluefruit.Advertising.setBeacon( elBeacon );

  Bluefruit.Advertising.restartOnDisconnect(true);  // Reiniciar en desconexión
  Bluefruit.Advertising.setInterval(32, 244);       // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);         // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                   // 0 = Don't stop advertising after n seconds
}