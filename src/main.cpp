#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "Graph.h"
#include <optional>
#include <iostream>

int main() {
    // 40x30 celdas pa q entren 100 estaciones
    size_t rows = 30;
    size_t columns = 40;
    size_t cellSize = 30; 

    sf::Vector2u windowSize(columns * cellSize, rows * cellSize);
    sf::RenderWindow window(sf::VideoMode(windowSize), "TransitOpt - Hito 3");
    window.setFramerateLimit(60);

    using MyGrid = CGrid<GridTrait<TerrainType>>;
    MyGrid mapa(rows, columns, cellSize);

    using MyGraph = CGraph<StationTrait<int>>;
    MyGraph redMetro(10); 

    int nextStationId = 1;
    int selectedStationId = -1; // -1 significa q no hay ninguna seleccionada
    int bridgesLeft = 3; // tenemos 3 puentes

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // si asen click isquierdo
            if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButton->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    sf::Vector2u gridCoords = mapa.GetGridCoords(mousePos.x, mousePos.y);
                    
                    // buscamos la estasion en la q hiso click
                    auto* clickedStation = redMetro.FindStationAt(gridCoords.x, gridCoords.y);
                    
                    if (clickedStation != nullptr) {
                        // si ya hay una seleccionada, intentamos conectar
                        if (selectedStationId != -1 && selectedStationId != clickedStation->GetId()) {
                            
                            // buscamos las dos estaciones pa sacar sus posiciones
                            auto* stA = redMetro.GetStationById(selectedStationId);
                            auto* stB = clickedStation;
                            
                            if (stA && stB) {
                                bool cruzaRio = mapa.IsRiverCrossing(stA->GetPixelPos().x, stA->GetPixelPos().y, stB->GetPixelPos().x, stB->GetPixelPos().y);
                                
                                if (cruzaRio) {
                                    if (bridgesLeft > 0) {
                                        bridgesLeft--; // gastamos un puente
                                        redMetro.AddEdge(selectedStationId, clickedStation->GetId(), true);
                                        std::cout << "via con puente creada. quedan: " << bridgesLeft << "\n";
                                    } else {
                                        std::cout << "no hay puentes suficientes!\n";
                                    }
                                } else {
                                    // via normal sin puente
                                    redMetro.AddEdge(selectedStationId, clickedStation->GetId(), false);
                                }
                            }
                            selectedStationId = -1; // deseleccionamos
                        } 
                        else {
                            // seleccionamos la estasion
                            selectedStationId = clickedStation->GetId();
                        }
                    } else {
                        // si hiso click en el vacio, creamos estasion y desseleccionamos
                        if (redMetro.FindStationAt(gridCoords.x, gridCoords.y) == nullptr) {
                            sf::Vector2f stationPixelPos(
                                (gridCoords.x * cellSize) + (cellSize / 2.0f),
                                (gridCoords.y * cellSize) + (cellSize / 2.0f)
                            );
                            redMetro.AddStation(nextStationId++, gridCoords.x, gridCoords.y, stationPixelPos);
                            selectedStationId = -1; 
                        }
                    }
                }
            }
        }

        window.clear();
        mapa.Draw(window);

        // dibujamos las vias
        for (size_t i = 0; i < redMetro.GetNumEdges(); ++i) {
            CEdge edge = redMetro.GetEdge(i);
            auto* stA = redMetro.GetStationById(edge.m_idStationA);
            auto* stB = redMetro.GetStationById(edge.m_idStationB);
            
            if (!stA || !stB) continue;

            // dibujamos la linea de la via
            sf::VertexArray via(sf::PrimitiveType::Lines, 2);
            via[0].position = stA->GetPixelPos();
            via[1].position = stB->GetPixelPos();
            
            if (edge.m_isBridge) {
                via[0].color = sf::Color::Red; // pintamos rojo si es puente
                via[1].color = sf::Color::Red;
            } else {
                via[0].color = sf::Color::Blue; // azul si es normal
                via[1].color = sf::Color::Blue;
            }
            window.draw(via);
        }

        // dibujamos las estaciones
        sf::CircleShape stationShape(cellSize / 2.5f);
        stationShape.setOrigin(sf::Vector2f(stationShape.getRadius(), stationShape.getRadius()));
        stationShape.setOutlineThickness(2.0f);
        stationShape.setOutlineColor(sf::Color::Black);

        for (size_t i = 0; i < redMetro.GetNumStations(); ++i) {
            auto* station = redMetro.GetStation(i);
            stationShape.setPosition(station->GetPixelPos());
            
            // si esta seleccionada, la pintamos verde
            if (station->GetId() == selectedStationId) {
                stationShape.setFillColor(sf::Color::Green);
            } else {
                stationShape.setFillColor(sf::Color::White);
            }
            window.draw(stationShape);
        }

        window.display();
    }

    return 0;
}