# Caso Enunciado:
## CasoBlueTide
Un acuario en calma, un pequeño universo acuático que respira en equilibrio, una ventana diaria para admirar la vida marina en su mejor forma. Y justo en ese punto donde la temperatura, el nivel de agua y la alimentación se sincronizan, aparece esa sensaci ón de conexión con la naturaleza que no tiene precio.
Movidos por esa idea y por una búsqueda constante de innovación, BlueTide dedicó meses de experimentación con distintos sensores y arquitecturas electrónicas, con el objetivo de construir un sistema inteligente de mantenimiento de acuarios que fuera confíable y, a la vez, agradable a la vista. Como resultado, BlueTide lanzó al mercado ReefKeeper, un dispositivo que integra electrónica embebida de última generación para llevar el cuidado automático de acuarios domésticos a otro nivel. Como parte de la expansión de su catálogo, la compañía se encuentra desarrollando prototipos de su nueva línea ReefKeeper Pro. Solution Features ReefKeeper Pro está construido sobre una placa ESP32 y contempla los siguientes componentes principales:
* Una pantalla LCD - 1602 (https://docs.wokwi.com/parts/wokwi-lcd1602).
* Dos pulsadores táctiles (tactile switch) de 6mm (https://docs.wokwi.com/parts/wokwi-pushbutton-6mm).
* Un sensor de temperatura DHT22 (https://docs.wokwi.com/parts/wokwi-dht22).
* Dos LEDs (https://docs.wokwi.com/parts/wokwi-led) - uno azul para indicar condiciones óptimas y uno amarillo para alertas.
* Un sensor ultrasónico HC-SR04 (https://docs.wokwi.com/parts/wokwi-hc-sr04) para medir el nivel de agua.
* Un Real-Time-Clock DS1307 (https://docs.wokwi.com/parts/wokwi-ds1307).
* Un relay module (https://docs.wokwi.com/parts/wokwi-relay-module) para controlar el calentador de agua.
* Un servo motor (https://docs.wokwi.com/parts/wokwi-servo) para controlar el dispensador automático de alimento.


## Comportamiento del device:
* En la esquina superior izquierda de la pantalla se muestra la temperatura actual del agua (°C), leída por el sensor DHT22. En la esquina inferior izquierda se presenta la temperatura objetivo configurada por el usuario para sus peces. Mediante los botones (+) y (-) el usuario puede aumentar o disminuir ese valor. El rango permitido va de 22.0°C a 28.0°C, y cada pulsación modifica el valor en pasos de 0.5°C. BlueTide señala que ReefKeeper está pensado para acuarios de entre 50 y 200 litros.
* El sistema evalúa continuamente la temperatura del agua frente al valor objetivo, con una tolerancia de ±1.0°C. Si la temperatura está más de 1.0°C por debajo del objetivo, el relay del calentador se activa, la pantalla muestra el estado CALIENTE en la esquina superior derecha, el LED azul se apaga y el LED amarillo se enciende. Si la temperatura se encuentra dentro del rango objetivo, el relay del calentador se apaga, la pantalla muestra OPTIMAL y el LED azul se enciende. Si la temperatura supera el rango objetivo, el relay del calentador también permanece apagado, mientras la pantalla muestra FRIO y el LED azul se mantiene apagado.
* El servomotor controla el dispensador de alimento: a las 8:00 AM y a las 6:00 PM, según la hora indicada por el RTC, el servo se posiciona en 90 grados durante 3 segundos (dispensando alimento) y luego regresa a 0 grados. Modos de operación Cuando el sensor ultrasónico HC-SR04 detecta que el nivel de agua es bajo (distancia mayor a 25 cm entre el sensor y la superficie del agua), el dispositivo entra en modo NIVEL AGUA Alert: el calentador se desactiva, el dispensador de alimento se bloquea, la pantalla muestra "NIVEL AGUA ALERT", el LED amarillo parpadea y se suspende la verificación de las reglas de temperatura. 
* Cuando el sensor detecta que el nivel de agua vuelve a ser adecuado(distancia menor a 25 cm), el dispositivo regresa a NORMAL MODE, retomando la verificación de las reglas de temperatura, restableciendo la pantalla normal, actualizando el estado de los LEDs y controlando el calentador según corresponda.
* Actualización de estado El dispositivo debe enviar a consola cada 5 segundos información de estado de operación, mostrando una estructura en formato JSON conteniendo: 

        deviceMacAddress (MAC Address del Dispositivo)
        operationMode (NIVEL_AGUA_ALERT / NORMAL_MODE)
        targetTemperature (Valor en °C) 
        currentTemperature (Valor en °C) 
        nivelAgua (Distancia en cm) 
        EstadoCalentado (CALENTANDO, OPTIMO, ENFRIANDO)
        createdAt (Fecha y hora actual)

## Technical constraints:
* El prototipo debe elaborarse con Wokwi y la programación debe ser con Arduino sketch (C++) aplicando Object-Oriented Programming y basado en ModestIoT para el Embedded Application, creando clases y objetos para representar el estado y comportamiento de los diversos tipos de sensors y actuators en el device (cuando ModestIoT no incluya tipos que ofrezcan dichas características).
* El contenido de sketch.ino debería enfocarse en la generación de instancia de una clase que sea descendiente de Device, e invocar al comportamiento correspondiente en los métodos setup() y loop().
* Aplique convenciones de nomenclatura en inglés, con principios, patrones y buenas prácticas de diseño de software orientado a objetos.
* Declare constantes para evitar los magic string y magic number anti-patterns.
* Documente con comentarios en inglés sus archivos de código fuente con las convenciones de Doxygen (ver Doxygen: Documenting the code en referencias).
* El MAC Address debe ser personalizado, utilizando un generador aleatorio de MAC Address donde el MAC Address Prefix sean los 4 últimos dígitos de su código de estudiante (ver utilidad en línea en referencias adicionales). El simulador de ESP32 cuenta con el atributo macAddress para asignar un valor personalizado (ver ESP32 Simulator | Changing the MAC address en referencias adicionales).
* Aplique convenciones de nomenclatura en inglés, con principios, patrones y buenas prácticas de diseño de software orientado a objetos, aplicando el approach de ModestIoT incluyendo descendientes de Sensor, Actuator según se requiera, con Commands, Events, Command Handlers, Query Handlers.
* Documente con comentarios en inglés cada uno de sus archivos de código fuente. Además incluya title blocks (ver lo referente a Explanation of Code en el Arduino Style Guide for Writing Content en la sección de Referencias).
* Al inicio de la ejecución debe mostrarse en consola la información de la empresa, así como sus nombres y apellidos como developer miembro del equipo de la empresa del caso. Considere en su sketch el código necesario para describir y demostrar los modos de operación y actualización de estado requeridos en el IoT Device.

## Instrucciones de Envio:
* Exporte el proyecto de Wokwi y empaquete el mismo como un archivo .zip. (único formato válido) con el nombre eb<NRC>u<código-estudiante>.zip (por ejemplo, eb17755u201621873.zip).
* Suba dos archivos: El archivo upc-pre-202610-1asi0572-examen-final-file_v1.pptx con sus respuestas (renombrado como eb<NRC>u<código-estudiante>-file.pptx, por ejemplo eb17755u201621873-file.pptx,sin empaquetar) y el archivo .zip con el proyecto de solución de Wokwi, en la Actividad indicada para el Examen final. 
* En Wokwi, el archivo libraries.txt debe contener:

        # Wokwi Library List
        # See https://docs.wokwi.com/guides/libraries
        ArduinoJson
        PubSubClient
        ESP32Servo
        LiquidCrystal I2C
        DHT sensor library
        Adafruit Unified Sensor
        EspSoftwareSerial
        HX711 Arduino Library
        ModestIoT@wokwi:fb939e3848beed149e380d7392ca7967aa440c62

## Preguntas Examen:
En base al caso:

Pregunta 1 (5 p.).
Bajo el marco de los 12 pasos del IoT System Design Steps (ver Anexo A), elabore el Definition of application service layer requirements (requisitos relacionados con la capa de servicio de la aplicación). Aquí considere a) Definición de la interfaz de usuario para cada servicio prestado (dependiendo del servicio la aplicación para dicho servicio podría requerir una interfaz gráfica para web, móvil, o tablet, o desktop, o consola, o más de un tipo).

Pregunta 2 (15 p.)
Cree un nuevo proyecto en Wokwi y desarrolle un prototipo que satisfaga los requisitos planteados y technical constraints para el Embedded Application del IoT Device solicitado en el caso.

Referencias
Wokwi Parts:
https://docs.wokwi.com/guides/esp32 
https://docs.wokwi.com/guides/esp32-wifi
https://docs.wokwi.com/parts/wokwi-dht22 
https://docs.wokwi.com/parts/wokwi-lcd1602 
https://docs.wokwi.com/parts/wokwi-pushbutton-6mm 
https://docs.wokwi.com/parts/wokwi-led 
https://docs.wokwi.com/parts/wokwi-hc-sr04 
https://docs.wokwi.com/parts/wokwi-relay-module 
https://docs.wokwi.com/parts/wokwi-ds1307 
https://docs.wokwi.com/parts/wokwi-servo 
Wokwi projects de referencia:
https://wokwi.com/projects/322410731508073042 
https://wokwi.com/projects/344891772964438612 
https://wokwi.com/projects/305193627138654786  
https://wokwi.com/projects/323706614646309460 
https://wokwi.com/projects/377396146936355841 
Wokwi Libraries (buscar por nombre en Wokwi Library Manager):
ArduinoJson
OneWire

