#include <Servo.h>

#include <SPI.h>
#include <Ethernet.h>

/**************************************************/

/*inicio variables para el servo motor*/

//objeto para manejar el motor
Servo miServo;

//pin para el servo
const int servoPin = 9;

//angulo que toma el motor
//90∞ -> puerta cerrada
//0∞  -> puerta abierta
int angulo;

/*fin variables para el servo motor*/

/**************************************************/

/*inicio variables para el sensor de sonido*/

//es el volumen analogico que lee el sensor
int volumen;

//contador para la cantidad de palmadas
int cantPalmadas;

//vector con los tiempos entre palmadas
int tiemposEntrePalmadas[5];

//indice para moverme en el vector
int i;

//tolerancia para cada tiempo entre palmadas
int tolerancia;

//tiempo en que se dio la ultima palmada
int tiempoUltimaPalmada;

//tiempo real desde la ultima palmada hasta el momento
int tiempoActualEntrePalmada;

//cantidad de palmadas totales a realizar
int palmadasTotales;

/*fin variables para el sensor de sonido*/

/**************************************************/

/*inicio variables para las luces de led*/

//pines para cada led
const int ledRojoPin = 5;
const int ledAmarilloPin = 6;
const int ledVerdePin = 7;

/*fin variables para las luces de led*/

/**************************************************/

/*inicio variables para reed switch*/

//pin para el reed switch
const int pinSwitch = 2;

//estado para leer y guardar el estado del reed switch
int estadoSwitch;

/*fin variables para reed switch*/

/**************************************************/


/**************************************************/
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   //Direccion Fisica MAC
byte ip[] = { 192, 168, 0, 50 };                       // IP Local que usted debe configurar 
byte gateway[] = { 192, 168, 0, 1 };                   // Puerta de enlace
byte subnet[] = { 255, 255, 255, 0 };                  //Mascara de Sub Red
EthernetServer server(80);                             //Se usa el puerto 80 del servidor     
String readString;
/**************************************************/


void setup() {

  //inicializo el serial por si se quiere imprimir algun valor o mensaje por el serial monitor (que seria como una consola)
  Serial.begin(9600);
  
  //inicializo los pines de los leds como output para encender las luces
  pinMode(ledRojoPin, OUTPUT);
  pinMode(ledAmarilloPin, OUTPUT);
  pinMode(ledVerdePin, OUTPUT);

  //inicializo el pin del reed switch como de entrada
  pinMode(pinSwitch, INPUT);   

  //lee el valor del estado del reed switch para saber si el sistema arranca con la puerta cerrada (que seria lo mas comun) o abierta.
  estadoSwitch = digitalRead(pinSwitch);


  Ethernet.begin(mac, ip, gateway, subnet); // Inicializa la conexion Ethernet y el servidor
  server.begin();
  Serial.print("El Servidor es: ");
  Serial.println(Ethernet.localIP());    // Imprime la direccion IP Local


  if (estadoSwitch == LOW) {        //puerta CERRADA

    inicializarVariables();

    cerrarPuerta();

    digitalWrite(ledAmarilloPin, HIGH);


  }else{                            //puerta ABIERTA
    
    abrirPuerta();

    //no inicializo las variables, ya que esto lo voy a hacer cuando la puerta se cierre
    
  }

}


void loop() {

  // Crea una conexion Cliente
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {   
      if (client.available()) {
        char c = client.read();
     
        //Lee caracter por caracter HTTP
        if (readString.length() < 100) {
          //Almacena los caracteres a un String
          readString += c;
          
         }

         // si el requerimiento HTTP fue finalizado
         if (c == '\n') {          
           Serial.println(readString); //Imprime en el monitor serial
     
           client.println("HTTP/1.1 200 OK");           //envia una nueva pagina en codigo HTML
           client.println("Content-Type: text/html");
           client.println();     
   
           delay(1);
           //detiene el cliente servidor
           client.stop();
           
        
           if (readString.indexOf("?dooropen") >0){
               abrirPuerta();
           }
           if (readString.indexOf("?doorclose") >0){
               cerrarPuerta();
           }
           
            // Limpia el String(Cadena de Caracteres para una nueva lectura
            readString="";  
           
         }
       }
    }
  }



  //constantemente leemos el estado del reed switch ya que la deteccion de sonido para abrir la puerta solo deberia funcionar cuando esta este cerrada
  estadoSwitch = digitalRead(pinSwitch);


  if (estadoSwitch == LOW) {        //puerta CERRADA

    //leo el nivel de sonido detectado analogicamente por la entrada A0
    volumen = analogRead(A0);
  
    if (volumen > 1000 && cantPalmadas == 0){

      //Serial.println(volumen);
      digitalWrite(ledAmarilloPin, LOW);
      cantPalmadas++;
      volumen = 0;
      delay(50);
      digitalWrite(ledAmarilloPin, HIGH);
      tiempoUltimaPalmada = millis();
      
    }  
  
    if ( cantPalmadas > 0 ){ 
      tiempoActualEntrePalmada = millis() - tiempoUltimaPalmada;
    }
  
    if ( volumen > 1000 && cantPalmadas > 0 ){

      if ( ( tiempoActualEntrePalmada > tiemposEntrePalmadas[i] - tolerancia ) && ( tiempoActualEntrePalmada < tiemposEntrePalmadas[i] + tolerancia ) ){
        
        //Serial.println(volumen);
        digitalWrite(ledAmarilloPin, LOW);
        cantPalmadas++;
        delay(50);
        digitalWrite(ledAmarilloPin, HIGH);
        i++;
        tiempoUltimaPalmada = millis();
        
      }else{
        
        digitalWrite(ledAmarilloPin, LOW);
        digitalWrite(ledRojoPin, HIGH);
        delay(3000);
        
        inicializarVariables();
        
        digitalWrite(ledRojoPin, LOW);
        digitalWrite(ledAmarilloPin, HIGH);
      }
      
    }
  
    if (cantPalmadas == palmadasTotales){
  
      digitalWrite(ledAmarilloPin, LOW);
      digitalWrite(ledVerdePin, HIGH);
       
      abrirPuerta();

      //ponemos 5 segundos para que el tipo abra la puerta, si no la abre la cerradura se cerrara
      delay(5000);
      
      estadoSwitch = digitalRead(pinSwitch);
      
      if (estadoSwitch == LOW) {        //puerta CERRADA

        digitalWrite(ledVerdePin, LOW);
        
        inicializarVariables(); 
        cerrarPuerta();
        digitalWrite(ledAmarilloPin, HIGH);
      
      }

    }
  
  } else {                            //puerta ABIERTA

    digitalWrite(ledAmarilloPin, LOW);
    digitalWrite(ledVerdePin, HIGH);

    //mientras la puerta este abierta me quedo loopeando, hasta que la puerta se cierra
    while(estadoSwitch == HIGH){
      estadoSwitch = digitalRead(pinSwitch);
    }

    digitalWrite(ledVerdePin, LOW);
    
    inicializarVariables();
    cerrarPuerta();
    digitalWrite(ledAmarilloPin, HIGH);
  
  }

}

void inicializarVariables(){
  volumen = 0;
  cantPalmadas = 0;
  
  tiemposEntrePalmadas[0] = 400;
  tiemposEntrePalmadas[1] = 100;
  tiemposEntrePalmadas[2] = 400;
  tiemposEntrePalmadas[3] = 400;
  tiemposEntrePalmadas[4] = 100;
  i = 0;
  tolerancia = 150;
  tiempoUltimaPalmada = 0;
  tiempoActualEntrePalmada = 0;
  palmadasTotales = ( sizeof(tiemposEntrePalmadas)/sizeof(int) ) + 1;

//Serial.print("Cantidad de palmadas a hacer: ");
//Serial.println(palmadasTotales);

}

void abrirPuerta(){

  miServo.attach(servoPin);

  for(angulo = 96; angulo  >= 0; angulo  -= 1){
    miServo.write(angulo);
    delay(5);
  }

  miServo.detach();

  delay(100);
  
}

void cerrarPuerta(){

  miServo.attach(servoPin);
  
  for(angulo  = 0; angulo  <=96; angulo  +=1 ){
    miServo.write( angulo );
    delay(5);
  }

  miServo.detach();

  delay(100);
  
}

