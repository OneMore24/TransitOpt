#include <SFML/Graphics.hpp>
#include "Grid.h"
#include <optional>

int main() {
    // Ventana de 960x720. 
    // Cuadrícula de 20 columnas x 15 filas (celdas de 48x48 px)
    size_t rows = 15;
    size_t columns = 20;
    size_t cellSize = 48;

    // Corrección 1: VideoMode ahora requiere un sf::Vector2u
    sf::Vector2u windowSize(
        static_cast<unsigned int>(columns * cellSize), 
        static_cast<unsigned int>(rows * cellSize)
    );
    
    sf::RenderWindow window(sf::VideoMode(windowSize), "TransitOpt - Grid");
    window.setFramerateLimit(60);

    // Instanciamos nuestro Grid usando los Traits
    using MyGrid = CGrid<GridTrait<TerrainType>>;
    MyGrid mapa(rows, columns, cellSize);

    while (window.isOpen()) {
        // Corrección 2: pollEvent ya no recibe parámetros y devuelve un std::optional
        while (const std::optional event = window.pollEvent()) {
            
            // Corrección 3: Se usa event->is<>() en lugar de event.type ==
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear();
        mapa.Draw(window);
        window.display();
    }

    return 0;
}