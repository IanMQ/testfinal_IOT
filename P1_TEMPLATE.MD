# P1 Template

Este archivo sirve como plantilla para redactar la respuesta de la Pregunta 1 sobre **Definition of application service layer requirements**.

La idea es partir del caso, identificar qué servicios reales ofrece la aplicación al usuario y describir para cada uno:

- qué hace el servicio,
- qué interfaz necesita,
- qué dispositivo o medio usa,
- y qué comportamiento visible debe tener.

## Tabla modelo

|Application Service Layer Requirement | Especificación de requisitos|
|-|-|
|Servicio 1|Describe aquí el nombre del servicio, el objetivo funcional, la interfaz de usuario requerida y el comportamiento esperado.|
|Servicio 2|Describe aquí otro servicio distinto del sistema, indicando cómo interactúa el usuario con él y desde qué tipo de interfaz.|
|Servicio 3|Agrega un servicio adicional si el caso lo requiere, por ejemplo monitoreo, alertas, configuración o historial.|
|Servicio 4|Continúa agregando filas por cada servicio relevante identificado en el caso.|
|Servicio X|La aplicación debe permitir al usuario realizar una acción concreta del sistema mediante la interfaz definida en el caso, mostrando el resultado de forma clara y oportuna.|

## Cómo llenarla

1. Lee el caso completo y subraya las acciones que el usuario puede realizar o ver.
2. Convierte cada acción importante en un servicio de aplicación.
3. Para cada servicio, responde estas preguntas:
   - ¿Qué necesita hacer el usuario?
   - ¿Qué información debe mostrar el sistema?
   - ¿Qué interfaz corresponde: LCD, web, móvil, consola, panel físico, etc.?
   - ¿Hay algún rango, límite o condición especial?
4. Si el caso tiene sensores y actuadores, piensa en servicios como:
   - visualización,
   - configuración,
   - alertas,
   - control manual,
   - monitoreo,
   - telemetría,
   - historial,
   - autenticación, si aplica.
5. Escribe cada fila de forma concreta y directa, sin repetir detalles técnicos innecesarios.

## Cómo obtener la información

Busca la información en estas partes del enunciado:

- **Comportamiento del device**: normalmente aquí aparecen los servicios principales.
- **Technical constraints**: aquí se ve qué tipo de app o interfaz espera el ejercicio.
- **Instrucciones de envío**: a veces indican el formato o medio de salida.
- **Lista de componentes**: ayuda a inferir qué servicios son posibles.

También puedes extraer servicios observando:

- qué datos se muestran en pantalla,
- qué controles tiene el usuario,
- qué alertas existen,
- qué eventos se reportan,
- y qué decisiones toma el sistema automáticamente.

## Ejemplo de razonamiento

Si un caso dice:

- “el usuario puede subir o bajar un valor con botones”

entonces el servicio puede ser:

- **Ajuste de parámetro**

y la interfaz podría ser:

- **botones físicos + LCD**

Si un caso dice:

- “el sistema envía estado cada cierto tiempo”

entonces el servicio puede ser:

- **telemetría**

y la interfaz podría ser:

- **consola serial** o **panel web**, según lo que pida el caso.

## Regla práctica

Un buen servicio de capa de aplicación normalmente tiene esta forma:

- nombre claro,
- propósito concreto,
- interfaz específica,
- y una condición observable.

Si no puedes explicar un servicio en una sola frase, probablemente está muy amplio y conviene dividirlo en dos.
