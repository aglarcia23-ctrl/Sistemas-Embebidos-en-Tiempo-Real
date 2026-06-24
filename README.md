Alan Efrén García Ortiz 
9539
IECSA-06B

Las prácticas se encuentran en las branches de este repositorio. En el archivo principal
hay un archivo main.cpp el cual contiene un código en c++ de un problema sencillo.
Favor de ignorar dicho código, fue con el propósito de crear este repositorio.
Los archivos calificables están en las branches de este repositorio.

A continuación se muestran las conclusiones de cada práctica

Práctica 1: Se comprobó la existencia e importancia del Idle Hook por medio de las prioridades y 
se estudió lo vital de poner variables tipo volatile

Práctica 2: Validó el diseño modular y la reutilización de código mediante pvParameters junto 
a estructuras estáticas, demostrando cómo la apropiación afecta el consumo de memoria 
(watermark) de cada tarea.  

Práctica 3: Demostró la superioridad de un Task Manager centralizado mediante TaskHandle_t, 
logrando transiciones seguras e instantáneas entre los estados Blocked y Suspended sin 
desperdiciar ciclos de CPU haciendo polling.  

Práctica 4: Consolidó una arquitectura determinista donde las colas garantizan una comunicación 
thread-safe libre de colisiones, mientras el TaskManager orquesta la máquina de estados basándose 
en señales suavizadas por el filtro de mediana. 
