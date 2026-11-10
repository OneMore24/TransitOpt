#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "Graph.h"
#include "Passenger.h"
#include <optional>
#include <iostream>
#include <algorithm>
#include <cmath>

int main() {
    size_t rows = 30, columns = 40, cellSize = 30; 
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(columns * cellSize, rows * cellSize)), "TransitOpt - Hito 4");
    window.setFramerateLimit(60);

    using MyGrid = CGrid<GridTrait<TerrainType>>;
    MyGrid mapa(rows, columns, cellSize);

    using MyGraph = CGraph<StationTrait<int>>;
    MyGraph redMetro(10); 

    int nextStationId = 1;
    int selectedStationId = -1;
    int bridgesLeft = 3;

    // lista de pasajeros
    std::vector<CPassenger> pasajeros;
    int nextPassengerId = 1;

    sf::Clock clock; // pa calcular el deltaTime

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            // click isquierdo = crear/conectar estaciones
            if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                sf::Vector2u gridCoords = mapa.GetGridCoords(mousePos.x, mousePos.y);
                auto* clickedStation = redMetro.FindStationAt(gridCoords.x, gridCoords.y);

                if (mouseButton->button == sf::Mouse::Button::Left) {
                    if (clickedStation != nullptr) {
                        if (selectedStationId != -1 && selectedStationId != clickedStation->GetId()) {
                            auto* stA = redMetro.GetStationById(selectedStationId);
                            auto* stB = clickedStation;
                            if (stA && stB) {
                                bool cruzaRio = mapa.IsRiverCrossing(stA->GetPixelPos().x, stA->GetPixelPos().y, stB->GetPixelPos().x, stB->GetPixelPos().y);
                                if (cruzaRio) {
                                    if (bridgesLeft > 0) { bridgesLeft--; redMetro.AddEdge(selectedStationId, clickedStation->GetId(), true); }
                                    else std::cout << "no hay puentes!\n";
                                } else {
                                    redMetro.AddEdge(selectedStationId, clickedStation->GetId(), false);
                                }
                            }
                            selectedStationId = -1;
                        } else {
                            selectedStationId = clickedStation->GetId();
                        }
                    } else {
                        if (redMetro.FindStationAt(gridCoords.x, gridCoords.y) == nullptr) {
                            sf::Vector2f pos((gridCoords.x * cellSize) + (cellSize / 2.0f), (gridCoords.y * cellSize) + (cellSize / 2.0f));
                            redMetro.AddStation(nextStationId++, gridCoords.x, gridCoords.y, pos);
                            selectedStationId = -1; 
                        }
                    }
                }
                
                // click derecho = crear pasajero
                if (mouseButton->button == sf::Mouse::Button::Right) {
                    if (clickedStation != nullptr && redMetro.GetNumStations() > 1) {
                        // elegimos un destino aleatorio diferente al origen
                        int targetId = clickedStation->GetId();
                        while(targetId == clickedStation->GetId()) {
                            int randIdx = rand() % redMetro.GetNumStations();
                            targetId = redMetro.GetStation(randIdx)->GetId();
                        }
                        
                        // usamos dijkstra
                        std::vector<int> path = redMetro.FindShortestPath(clickedStation->GetId(), targetId);
                        
                        if (!path.empty()) {
                            pasajeros.emplace_back(nextPassengerId++, clickedStation->GetId(), targetId, path, clickedStation->GetPixelPos());
                            std::cout << "pasajero " << nextPassengerId-1 << " creado. ruta de " << clickedStation->GetId() << " a " << targetId << "\n";
                        } else {
                            std::cout << "no hay ruta disponible!\n";
                        }
                    }
                }
            }
        }

        // actualizamos a los pasajeros
        for (auto& p : pasajeros) {
            p.Update(deltaTime, &redMetro);
        }

        // borramos a los q ya llegaron a su destino pa no llenar la ram
        pasajeros.erase(
            std::remove_if(pasajeros.begin(), pasajeros.end(),
                [](const CPassenger& p) { return p.HasArrived(); }),
            pasajeros.end()
        );

        window.clear();
        mapa.Draw(window);

        // dibujamos vias
        for (size_t i = 0; i < redMetro.GetNumEdges(); ++i) {
            CEdge edge = redMetro.GetEdge(i);
            auto* stA = redMetro.GetStationById(edge.m_idStationA);
            auto* stB = redMetro.GetStationById(edge.m_idStationB);
            if (!stA || !stB) continue;
            sf::VertexArray via(sf::PrimitiveType::Lines, 2);
            via[0].position = stA->GetPixelPos(); via[1].position = stB->GetPixelPos();
            sf::Color color = edge.m_isBridge ? sf::Color::Red : sf::Color::Blue;
            via[0].color = color; via[1].color = color;
            window.draw(via);
        }

        // dibujamos estaciones
        sf::CircleShape stationShape(cellSize / 2.5f);
        stationShape.setOrigin(sf::Vector2f(stationShape.getRadius(), stationShape.getRadius()));
        stationShape.setOutlineThickness(2.0f);
        stationShape.setOutlineColor(sf::Color::Black);
        for (size_t i = 0; i < redMetro.GetNumStations(); ++i) {
            auto* station = redMetro.GetStation(i);
            stationShape.setPosition(station->GetPixelPos());
            stationShape.setFillColor(station->GetId() == selectedStationId ? sf::Color::Green : sf::Color::White);
            window.draw(stationShape);
        }

        // dibujamos pasajeros
        for (auto& p : pasajeros) p.Draw(window);

        window.display();
    }
    return 0;
}