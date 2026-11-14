
# TransitOpt 🚇🧠

Motor de optimización de redes de transporte (Network Designer) utilizando **Algoritmos Genéticos** y **Programación Concurrente**, desarrollado en C++17.

El usuario define el escenario (estaciones, río y límites de puentes y rutas) y la Inteligencia Artificial calcula la topología de red más eficiente, conectando las estaciones más cercanas y minimizando el uso de puentes y líneas de metro.

## ⚙️ Requisitos del Sistema

*   **Lenguaje:** C++17 o superior.
*   **Librerías:** 
    *   [SFML 3.0](https://www.sfml-dev.org/) (Gráficos, Ventana, Audio).
    *   [nlohmann/json](https://github.com/nlohmann/json) (Cabecera única para leer configuración).
*   **Compatibilidad:** El código ha sido desarrollado y probado en **Linux (Kubuntu)**. Sin embargo, al usar C++ estándar y SFML, es **totalmente compatible con Windows** y macOS.
    *   *Nota para Windows:* Para compilar en Windows, puedes usar MinGW (`mingw32-make`) o Visual Studio. Recuerda cambiar la ruta de la fuente en `Game.cpp` (ej. `"C:/Windows/Fonts/arial.ttf"`).

Instalación de dependencias en Ubuntu/Kubuntu:
```bash
sudo apt update
sudo apt install build-essential libsfml-dev
# Descargar json.hpp
wget -P include/ https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
```

## 🚀 Compilación y Ejecución

El proyecto incluye un `Makefile` para automatizar la compilación de todos los módulos en Linux.

```bash
# 1. Limpiar compilaciones anteriores
make clean

# 2. Compilar el proyecto
make

# 3. Ejecutar el simulador
make run
```
*(Al ejecutarse por primera vez, la terminal te pedirá configurar los parámetros iniciales: rutas máximas, puentes, población de la IA y generaciones).*

## 🎮 Controles del Simulador

| Acción | Control |
| :--- | :--- |
| **Crear Estación** | Arrastrar (Drag & Drop) una figura desde el menú izquierdo al mapa. |
| **Pintar Río** | Tecla `R` (mantener clic izquierdo y arrastrar para dibujar agua). |
| **Diseñar Red (IA)** | Tecla `G` (Genera la red óptima conectando estaciones cercanas). |
| **Limpiar Mapa** | Tecla `C`. |
| **Guardar Mapa** | Tecla `S`. |
| **Cargar Mapa** | Tecla `L`. |

## 📂 Estructura del Proyecto

*   `include/` y `src/`: Código fuente modular.
    *   `GeneticAlgorithm`: Motor de IA con hilos (`std::async`) y optimización evolutiva.
    *   `Grid`: Cuadrícula del mapa y detección de ríos (Algoritmo de Bresenham).
    *   `Graph`: Estructura de grafos para las conexiones de la red.
    *   `Line`: Representación de las rutas de metro.
    *   `Renderer`: Interfaz gráfica separada de la lógica de negocio.
*   `config.json`: Archivo de configuración de parámetros por defecto y playlist de música.
*   `musica/`: Carpeta donde debes colocar tus canciones en formato **`.ogg`** o **`.wav`** para que suenen mientras la IA calcula.

## 🧠 Características Técnicas

*   **IA Evolutiva (Topológica):** Genera miles de redes posibles, cruza las mejores y muta para encontrar la combinación que conecta más estaciones usando la menor cantidad de vías (distancia) y rutas posibles.
*   **Concurrencia:** El cálculo de la IA corre en un hilo secundario (`std::future`) para no congelar la interfaz gráfica (60 FPS estables). Uso de *Thread Pool* para no saturar la CPU.
*   **Manejo de Memoria:** Uso de arreglos 1D contiguos (`m_pData`) para las celdas del mapa, maximizando el *CPU Cache*.
*   **Persistencia:** Guardado y carga del ecosistema del mapa en formato JSON.

## 🤝 Contacto y Contribuciones

Este es un proyecto en constante evolución. Si tienes ideas para mejorar el algoritmo, optimizar el código o encuentras algún bug, ¡me encantaría saberlo!

*   **GitHub:** [OneMore24](https://github.com/OneMore24)
*   **Correo:** *alanjesus1946@outlook.com*
*   **¿Ideas o mejores formas de hacerlo?:** Si crees que hay una arquitectura más eficiente, un algoritmo genético mejor o una forma más limpia de manejar los hilos, siéntete libre de abrir un **Issue** o enviar un **Pull Request**. Toda retroalimentación constructiva es bienvenida.

## 📝 Licencia
Este proyecto está bajo la Licencia MIT. Siéntete libre de usarlo, modificarlo y aprender de él.
```
