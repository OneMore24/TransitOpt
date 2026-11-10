#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "Graph.h"
#include <optional>

int main() {
    // 40x30 celdas pa q entren 100 estaciones
    size_t rows = 30;
    size_t columns = 40;
    size_t cellSize = 30; 

    sf::Vector2u windowSize(columns * cellSize, rows * cellSize);
    sf::RenderWindow window(sf::VideoMode(windowSize), "TransitOpt - Hito 2");
    window.setFramerateLimit(60);

    using MyGrid = CGrid<GridTrait<TerrainType>>;
    MyGrid mapa(rows, columns, cellSize);

    using MyGraph = CGraph<StationTrait<int>>;
    MyGraph redMetro(10); // capasidad inicial

    int nextStationId = 1;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // si asen click isquierdo
            if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButton->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    
                    // pasamos de pixel a cordenada de cuadricula
                    sf::Vector2u gridCoords = mapa.GetGridCoords(mousePos.x, mousePos.y);
                    
                    // si ya hay una estasion ahi no asemos nada
                    if (redMetro.FindStationAt(gridCoords.x, gridCoords.y) == nullptr) {
                        sf::Vector2f stationPixelPos(
                            (gridCoords.x * cellSize) + (cellSize / 2.0f),
                            (gridCoords.y * cellSize) + (cellSize / 2.0f)
                        );
                        redMetro.AddStation(nextStationId++, gridCoords.x, gridCoords.y, stationPixelPos);
                    }
                }
            }
        }

        window.clear();
        mapa.Draw(window);

        // dibujamos las estaciones
        sf::CircleShape stationShape(cellSize / 2.5f);
        // corregido pa q acepte el vector en sfml 3.0
        stationShape.setOrigin(sf::Vector2f(stationShape.getRadius(), stationShape.getRadius()));
        stationShape.setFillColor(sf::Color::White);
        stationShape.setOutlineThickness(2.0f);
        stationShape.setOutlineColor(sf::Color::Black);

        for (size_t i = 0; i < redMetro.GetNumStations(); ++i) {
            auto* station = redMetro.GetStation(i);
            stationShape.setPosition(station->GetPixelPos());
            window.draw(stationShape);
        }

        window.display();
    }

    return 0;
}