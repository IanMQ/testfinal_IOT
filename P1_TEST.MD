# Pregunta 1

Bajo el marco de los 12 pasos del IoT System Design Steps, los requisitos de la capa de servicio de la aplicación para **ReefKeeper Pro** se orientan a ofrecer al usuario local del acuario una interfaz simple, física y de fácil lectura, complementada con consola serial para monitoreo técnico y telemetría. El prototipo no requiere interfaz web, móvil ni tablet para cumplir el caso; la interacción principal se realiza desde la pantalla LCD y los botones del equipo.

|Application Service Layer Requirement | Especificación de requisitos|
|-|-|
|Servicio de visualización de temperatura actual y objetivo|La aplicación debe mostrar en una interfaz **física tipo LCD 1602** la temperatura actual del agua en la esquina superior izquierda y la temperatura objetivo en la esquina inferior izquierda. La interfaz debe ser de lectura inmediata y apta para uso directo en el equipo embebido.|
|Servicio de ajuste de temperatura objetivo|La aplicación debe permitir al usuario aumentar o disminuir la temperatura objetivo mediante **dos botones físicos** (+ y -). La interfaz asociada es **hardware local** y debe responder con una pulsación por incremento/decremento de 0.5 °C dentro del rango 22.0 °C a 28.0 °C.|
|Servicio de visualización del estado térmico|La aplicación debe informar el estado operativo del sistema de calefacción en la **LCD 1602**, mostrando el estado correspondiente a la regulación térmica del agua. Esta interfaz es local y de uso continuo para el usuario del acuario.|
|Servicio de alerta por nivel bajo de agua|La aplicación debe presentar una alerta visible cuando el HC-SR04 detecte nivel de agua bajo. La interfaz debe ser **LCD 1602** para el mensaje de alerta y **LED amarillo parpadeante** como indicador visual adicional.|
|Servicio de restablecimiento a modo normal|La aplicación debe notificar visualmente el retorno a modo normal cuando el nivel de agua vuelva a ser adecuado. La interfaz debe ser **LCD 1602** y **LEDs de estado** para reflejar la recuperación operativa.|
|Servicio de control de alimentación automática|La aplicación debe mostrar el estado del dispensador de alimento en la lógica del equipo embebido. La interfaz asociada es principalmente **consola serial** para trazas de operación y, de forma indirecta, el **servo** como actuador visible del proceso.|
|Servicio de monitoreo y actualización de telemetría|La aplicación debe enviar cada 5 segundos un registro de estado en formato JSON por **consola serial** y hacia el endpoint remoto definido. La interfaz de este servicio es **consola serial de depuración/monitoreo técnico**.|
|Servicio de arranque e identificación del dispositivo|La aplicación debe mostrar al iniciar la ejecución la información de la empresa, el desarrollador y el identificador del dispositivo mediante **consola serial**. Esta interfaz apoya la trazabilidad y verificación del prototipo.|

https://wokwi.com/projects/469740941197912065