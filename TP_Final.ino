#include <Servo.h>

#include <SPI.h>
#include <Ethernet.h>

#include <SD.h>

//Archivo que contendra los tiempos para el codigo de sonido
File myFile;

/**************************************************/

/*inicio variables para el servo motor*/

//objeto para manejar el motor
Servo miServo;

//pin para el servo
const int servoPin = 9;

//angulo que toma el motor
//100° -> puerta cerrada
//0°  -> puerta abierta
int angulo;

/*fin variables para el servo motor*/

/**************************************************/

/*inicio variables para el sensor de sonido*/

//es el volumen analogico que lee el sensor
int volumen;

//contador para la cantidad de palmadas
int cantPalmadas;

//vector con los tiempos entre palmadas
int tiemposEntrePalmadas[10];

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
//byte gateway[] = { 192, 168, 0, 1 };                   // Puerta de enlace
//byte subnet[] = { 255, 255, 255, 0 };                  //Mascara de Sub Red
EthernetServer server(80);                             //Se usa el puerto 80 del servidor     
EthernetClient client;
String readerString;
String palabraClave;
/**************************************************/

//0=cerrada , 1=abierta
int state;

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

  //al estar el modulo de la SD en el shield, este necesita tener libre el pin 4
  if (!SD.begin(4)) {
    Serial.println("fallo la inicializacion de la SD");
    while(1){
      digitalWrite(ledRojoPin, HIGH);
      delay(1000);
      digitalWrite(ledRojoPin, LOW);
      delay(1000);
    }
  }

//  Ethernet.begin(mac, ip, gateway, subnet); // Inicializa la conexion Ethernet y el servidor
  Ethernet.begin(mac, ip);
  server.begin();
//  Serial.print("El Servidor es: ");
//  Serial.println(Ethernet.localIP());    // Imprime la direccion IP Local


  if (estadoSwitch == LOW) {        //puerta CERRADA
    state = 0;
    inicializarVariables();

    cerrarPuerta();

    digitalWrite(ledAmarilloPin, HIGH);


  }else{                            //puerta ABIERTA
    state = 1;
    abrirPuerta();

    //no inicializo las variables, ya que esto lo voy a hacer cuando la puerta se cierre
    
  }

}


void loop() {

  // Crea una conexion Cliente
  client = server.available();
  
  if (client) {
//    Serial.println("new client");
    while (client.connected()) {   
      if (client.available()) {
        char c = client.read();
     
        //Lee caracter por caracter HTTP
        if (readerString.length() < 100) {
          //Almacena los caracteres a un String
          readerString += c;
          
         }

         // si el requerimiento HTTP fue finalizado
         if (c == '\n') {          
//           Serial.print("requerimiento http: "); 
//           Serial.println(readerString);

           palabraClave = readerString.substring(6,14);
//           Serial.print("palabra clave: ");
//           Serial.println(palabraClave);

           //pasamos el string a char *
           char * cstr = new char [palabraClave.length()+1];
           strcpy (cstr, palabraClave.c_str());

           //envia una nueva pagina en codigo HTML
           client.println("HTTP/1.1 200 OK");           
           client.println("Content-Type: text/html");
           client.println();     
   
           delay(50);
           //detiene el cliente servidor
           client.stop();
           
           if (strcmp (cstr,"dooropen") == 0){
//               Serial.println("abrir puerta");
               digitalWrite(ledAmarilloPin, LOW);
               digitalWrite(ledVerdePin, HIGH);

               abrirPuerta();
           }
           if (strcmp (cstr,"doorclos") == 0){
//               Serial.println("cerrar puerta");
               digitalWrite(ledVerdePin, LOW);
               inicializarVariables();
               cerrarPuerta();
               digitalWrite(ledAmarilloPin, HIGH);
           }
           
           if (strcmp (cstr,"password") == 0){
//               Serial.println("cambiar contraseña");
               String loQueLeo = readerString.substring(15,readerString.lastIndexOf("x"));
               grabarCodigo(loQueLeo);
               delay(1000);
               inicializarVariables();
               for(int i = 0; i < 3; i++){
                 digitalWrite(ledRojoPin, HIGH);
                 digitalWrite(ledAmarilloPin, HIGH);
                 digitalWrite(ledVerdePin, HIGH);
                 delay(1000);
                 digitalWrite(ledRojoPin, LOW);
                 digitalWrite(ledAmarilloPin, LOW);
                 digitalWrite(ledVerdePin, LOW);
                 delay(1000);
               }
               digitalWrite(ledAmarilloPin, HIGH);
           } 
                     
           // Limpia el String(Cadena de Caracteres para una nueva lectura
           readerString="";  
           
         }
       }
    }
  }



  //constantemente leemos el estado del reed switch ya que la deteccion de sonido para abrir la puerta solo deberia funcionar cuando esta este cerrada
  estadoSwitch = digitalRead(pinSwitch);


  if (estadoSwitch == LOW && state==0) {        //puerta CERRADA

    //leo el nivel de sonido detectado analogicamente por la entrada A0
    volumen = analogRead(A0);
  
    //solo para la primera palmada
    if (volumen > 1000 && cantPalmadas == 0){

      //Serial.println(volumen);
      digitalWrite(ledAmarilloPin, LOW);
      cantPalmadas++;
      volumen = 0;
      delay(100);
      digitalWrite(ledAmarilloPin, HIGH);
      tiempoUltimaPalmada = millis();
      
    }  
  
    if ( cantPalmadas > 0 ){ 
      tiempoActualEntrePalmada = millis() - tiempoUltimaPalmada;
    }
  
    if ( volumen > 1000 && cantPalmadas > 0 ){

      //validacion del tiempo entre los sonidos, que sea igual al de la correspondiente posicion en el vector, mas menos una tolerancia
      if ( ( tiempoActualEntrePalmada > tiemposEntrePalmadas[i] - tolerancia ) && ( tiempoActualEntrePalmada < tiemposEntrePalmadas[i] + tolerancia ) ){
        
        //Serial.println(volumen);
        digitalWrite(ledAmarilloPin, LOW);
        cantPalmadas++;
        delay(100);
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
    if (estadoSwitch == HIGH ){
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

}

void inicializarVariables(){
  
  volumen = 0;
  cantPalmadas = 0;


  myFile = SD.open("codigo.txt", FILE_READ);
  delay(100);

  if (myFile) {

    char inputString [10];
    char inputChar;
    int stringIndex = 0; // String stringIndexing int;
    i=0;
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) {

        //leo caracteres
        inputChar = myFile.read();

        if (inputChar != ','){
          inputString[stringIndex] = inputChar; // Store it
          stringIndex++; // Increment where to write next
        }else{
          Serial.print("test: "); // shows that the program is cycling, for debugging only
          tiemposEntrePalmadas[i] = atoi(inputString);
          Serial.println(tiemposEntrePalmadas[i]);
          i++;
          delay (50);
          stringIndex = 0; // clear the value for the next cycle
        }
              
    }
    
    // close the file:
    myFile.close();
    
  } else {
    
    Serial.println("error al abrir codigo.txt");
    while(1){
      digitalWrite(ledRojoPin, HIGH);
      delay(1000);
      digitalWrite(ledRojoPin, LOW);
      delay(1000);
    }
    
  }
  

  
  tolerancia = 300;
  tiempoUltimaPalmada = 0;
  tiempoActualEntrePalmada = 0;
  palmadasTotales = i+1;
  i = 0;
  
Serial.print("Cantidad de palmadas a hacer: ");
Serial.println(palmadasTotales);

}

void abrirPuerta(){
  state = 1;
  miServo.attach(servoPin);

  for(angulo = 100; angulo  >= 0; angulo  -= 1){
    miServo.write(angulo);
    delay(5);
  }

  miServo.detach();

  delay(100);
  
}

void cerrarPuerta(){
  state = 0;
  miServo.attach(servoPin);
  
  for(angulo  = 0; angulo  <=100; angulo  +=1 ){
    miServo.write( angulo );
    delay(5);
  }

  miServo.detach();

  delay(100);
  
}

void grabarCodigo(String cadena){

  if (SD.exists("codigo.txt")) {
    SD.remove("codigo.txt");
    Serial.println("archivo borrado.");
  }


  myFile = SD.open("codigo.txt", FILE_WRITE);
  
  Serial.print("la cadena a guardar es esta: ");
  Serial.println(cadena);

  if (myFile) {
    
    //tendria que escribir el substring tal cual como viene y concatenarle una coma al final
    myFile.print(cadena);
    myFile.println(",");
    
    // close the file:
    myFile.close();

  } else {
    
    Serial.println("errooooooor al abrir codigo.txt");
    while(1){
      digitalWrite(ledRojoPin, HIGH);
      delay(1000);
      digitalWrite(ledRojoPin, LOW);
      delay(1000);
    }
    
  }
  
}
