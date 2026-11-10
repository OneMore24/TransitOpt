#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "Graph.h"
#include "Passenger.h"
#include <optional>
#include <iostream>
#include <random>
#include <algorithm>

void DrawStationShape(sf::RenderWindow& window, StationType type, sf::Vector2f pos, float size, sf::Color color) {
    sf::CircleShape circle(size);
    circle.setOrigin(sf::Vector2f(size, size));
    circle.setPosition(pos);
    circle.setFillColor(color);
    circle.setOutlineThickness(3);
    circle.setOutlineColor(sf::Color::Black);

    sf::RectangleShape rect(sf::Vector2f(size * 2, size * 2));
    rect.setOrigin(sf::Vector2f(size, size));
    rect.setPosition(pos);
    rect.setFillColor(color);
    rect.setOutlineThickness(3);
    rect.setOutlineColor(sf::Color::Black);

    switch (type) {
        case StationType::CIRCULO: circle.setPointCount(30); window.draw(circle); break;
        case StationType::CUADRADO: window.draw(rect); break;
        case StationType::TRIANGULO: circle.setPointCount(3); window.draw(circle); break;
        case StationType::OVALO: circle.setPointCount(30); circle.setScale(sf::Vector2f(1.5f, 0.7f)); window.draw(circle); break;
        case StationType::PENTAGONO: circle.setPointCount(5); window.draw(circle); break;
        case StationType::CONO: circle.setPointCount(3); circle.setScale(sf::Vector2f(0.8f, 1.5f)); window.draw(circle); break;
        case StationType::DIAMANTE: circle.setPointCount(4); circle.setRotation(sf::degrees(45)); window.draw(circle); break;
        case StationType::ESTRELLA: {
            sf::ConvexShape star; star.setPointCount(10);
            for(int i=0; i<10; i++) {
                float angle = i * 36.0f * 3.14159f / 180.0f;
                float r = (i % 2 == 0) ? size : size * 0.4f;
                star.setPoint(i, sf::Vector2f(r * std::cos(angle), r * std::sin(angle)));
            }
            star.setOrigin(sf::Vector2f(0,0)); star.setPosition(pos); 
            star.setFillColor(color); star.setOutlineThickness(3); star.setOutlineColor(sf::Color::Black);
            window.draw(star); break;
        }
        case StationType::RECTANGULO: rect.setScale(sf::Vector2f(1.5f, 0.8f)); window.draw(rect); break;
        case StationType::CRUZ: {
            sf::RectangleShape r1(sf::Vector2f(size * 2, size * 0.7f)); r1.setOrigin(sf::Vector2f(size, size*0.35f)); r1.setPosition(pos); r1.setFillColor(color); r1.setOutlineThickness(2); r1.setOutlineColor(sf::Color::Black);
            sf::RectangleShape r2(sf::Vector2f(size * 0.7f, size * 2)); r2.setOrigin(sf::Vector2f(size*0.35f, size)); r2.setPosition(pos); r2.setFillColor(color); r2.setOutlineThickness(2); r2.setOutlineColor(sf::Color::Black);
            window.draw(r1); window.draw(r2); break;
        }
    }
}

int main() {
    size_t rows = 30, columns = 40, cellSize = 30; 
    int menuWidth = 120;
    bool modoRio = false; // pa saber si estamos pintando agua
    
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(columns * cellSize + menuWidth, rows * cellSize)), "TransitOpt - Hito 6");
    window.setFramerateLimit(60);

    using MyGrid = CGrid<GridTrait<TerrainType>>;
    MyGrid mapa(rows, columns, cellSize);

    using MyGraph = CGraph<StationTrait<int>>;
    MyGraph redMetro(10); 

    int nextStationId = 1;
    int selectedStationId = -1;
    int bridgesLeft = 3;
    std::vector<CPassenger> pasajeros;
    int nextPassengerId = 1;
    int draggingType = -1; 

    sf::Clock clock;
    StationType tipos[10] = {StationType::CIRCULO, StationType::CUADRADO, StationType::TRIANGULO, StationType::OVALO, StationType::PENTAGONO, StationType::CONO, StationType::DIAMANTE, StationType::ESTRELLA, StationType::RECTANGULO, StationType::CRUZ};

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            // usamos la vista por defecto siempre
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            // tecla R pa activar el modo rio
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::R) {
                    modoRio = !modoRio;
                    std::cout << "modo rio: " << (modoRio ? "ON" : "OFF") << "\n";
                }
            }

            // si el mouse se mueve y estamos en modo rio, pintamos agua
            if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                if (modoRio && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    if (mousePos.x >= menuWidth) {
                        sf::Vector2u gridCoords = mapa.GetGridCoords(mousePos.x - menuWidth, mousePos.y);
                        mapa.SetCell(gridCoords.x, gridCoords.y, TerrainType::AGUA);
                    }
                }
            }

            if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButton->button == sf::Mouse::Button::Left) {
                    // click en el menu
                    if (mousePos.x > 0 && mousePos.x < menuWidth) {
                        int index = mousePos.y / (window.getSize().y / 10.0f);
                        if (index >= 0 && index < 10) draggingType = static_cast<int>(tipos[index]);
                    } 
                    // click en el mapa (despues del menu)
                    else if (mousePos.x >= menuWidth) {
                        sf::Vector2u gridCoords = mapa.GetGridCoords(mousePos.x - menuWidth, mousePos.y);
                        
                        // si estamos en modo rio, pintamos agua
                        if (modoRio) {
                            mapa.SetCell(gridCoords.x, gridCoords.y, TerrainType::AGUA);
                        } else {
                            // logica normal de estaciones
                            auto* clickedStation = redMetro.FindStationAt(gridCoords.x, gridCoords.y);
                            if (clickedStation != nullptr) {
                                if (selectedStationId != -1 && selectedStationId != clickedStation->GetId()) {
                                    auto* stA = redMetro.GetStationById(selectedStationId);
                                    bool cruzaRio = mapa.IsRiverCrossing(stA->GetPixelPos().x, stA->GetPixelPos().y, clickedStation->GetPixelPos().x, clickedStation->GetPixelPos().y);
                                    if (cruzaRio) {
                                        if (bridgesLeft > 0) { bridgesLeft--; redMetro.AddEdge(selectedStationId, clickedStation->GetId(), true); }
                                    } else redMetro.AddEdge(selectedStationId, clickedStation->GetId(), false);
                                    selectedStationId = -1;
                                } else selectedStationId = clickedStation->GetId();
                            }
                        }
                    }
                }

                // click derecho = pasajero
                if (mouseButton->button == sf::Mouse::Button::Right && mousePos.x >= menuWidth && !modoRio) {
                    sf::Vector2u gridCoords = mapa.GetGridCoords(mousePos.x - menuWidth, mousePos.y);
                    auto* clickedStation = redMetro.FindStationAt(gridCoords.x, gridCoords.y);
                    if (clickedStation != nullptr && redMetro.GetNumStations() > 1) {
                        StationType targetType;
                        do { targetType = static_cast<StationType>(rand() % 10); } while (targetType == clickedStation->GetType());

                        std::vector<int> path = redMetro.FindShortestPathToType(clickedStation->GetId(), targetType);
                        if (!path.empty()) {
                            pasajeros.emplace_back(nextPassengerId++, clickedStation->GetId(), targetType, path, clickedStation->GetPixelPos());
                        }
                    }
                }
            }

            // soltamos el mouse pa crear la estasion
            if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseButton->button == sf::Mouse::Button::Left && draggingType != -1) {
                    if (mousePos.x >= menuWidth) {
                        sf::Vector2u gridCoords = mapa.GetGridCoords(mousePos.x - menuWidth, mousePos.y);
                        
                        // revisamos q no haya estasion ya y q no sea agua
                        if (redMetro.FindStationAt(gridCoords.x, gridCoords.y) == nullptr) {
                            if (mapa.GetCell(gridCoords.x, gridCoords.y) == TerrainType::AGUA) {
                                std::cout << "no se puede poner estasion en el agua!\n";
                            } else {
                                sf::Vector2f pos((gridCoords.x * cellSize) + (cellSize / 2.0f), (gridCoords.y * cellSize) + (cellSize / 2.0f));
                                redMetro.AddStation(nextStationId++, gridCoords.x, gridCoords.y, pos, static_cast<StationType>(draggingType));
                            }
                        }
                    }
                    draggingType = -1;
                }
            }
        }

        for (auto& p : pasajeros) p.Update(deltaTime, &redMetro);
        pasajeros.erase(std::remove_if(pasajeros.begin(), pasajeros.end(), [](const CPassenger& p) { return p.HasArrived(); }), pasajeros.end());

        window.clear(sf::Color(240, 240, 240));

        // --- DIBUJAMOS EL MAPA (con el offset del menu) ---
        mapa.Draw(window, menuWidth);

        for (size_t i = 0; i < redMetro.GetNumEdges(); ++i) {
            CEdge edge = redMetro.GetEdge(i);
            auto* stA = redMetro.GetStationById(edge.m_idStationA);
            auto* stB = redMetro.GetStationById(edge.m_idStationB);
            if (!stA || !stB) continue;
            
            sf::VertexArray via(sf::PrimitiveType::Lines, 2);
            // sumamos menuWidth a la posision visual de las estaciones
            via[0].position = sf::Vector2f(stA->GetPixelPos().x + menuWidth, stA->GetPixelPos().y); 
            via[1].position = sf::Vector2f(stB->GetPixelPos().x + menuWidth, stB->GetPixelPos().y);
            
            sf::Color c = edge.m_isBridge ? sf::Color::Red : sf::Color::Blue;
            via[0].color = c; via[1].color = c;
            window.draw(via);
        }

        for (size_t i = 0; i < redMetro.GetNumStations(); ++i) {
            auto* station = redMetro.GetStation(i);
            sf::Color fill = (station->GetId() == selectedStationId) ? sf::Color::Green : sf::Color::White;
            // sumamos menuWidth pa dibujar
            sf::Vector2f drawPos(station->GetPixelPos().x + menuWidth, station->GetPixelPos().y);
            DrawStationShape(window, station->GetType(), drawPos, cellSize / 3.0f, fill);
        }

        for (auto& p : pasajeros) {
            // los pasajeros tb se dibujan con offset
            sf::Vector2f pPos = p.GetPixelPos();
            sf::CircleShape c(4); c.setOrigin(sf::Vector2f(4,4));
            c.setPosition(sf::Vector2f(pPos.x + menuWidth, pPos.y));
            c.setFillColor(sf::Color::Red);
            window.draw(c);
        }

        // --- DIBUJAMOS EL MENU (al final pa q este arriba) ---
        sf::RectangleShape menuBg(sf::Vector2f(menuWidth, window.getSize().y));
        // si esta activo el modo rio, el menu se pone azul
        menuBg.setFillColor(modoRio ? sf::Color(100, 149, 237) : sf::Color(60, 60, 60));
        menuBg.setOutlineThickness(2);
        menuBg.setOutlineColor(sf::Color::Black);
        window.draw(menuBg);

        for (int i = 0; i < 10; ++i) {
            float posY = (i * window.getSize().y / 10.0f) + (window.getSize().y / 20.0f);
            sf::Vector2f pos(menuWidth / 2.0f, posY);
            DrawStationShape(window, tipos[i], pos, 15, sf::Color::White); 
        }

        // dibujamos el fantasma
        if (draggingType != -1) {
            sf::Vector2f currentMousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            DrawStationShape(window, static_cast<StationType>(draggingType), currentMousePos, cellSize / 3.0f, sf::Color(255, 255, 255, 200));
        }

        window.display();
    }
    return 0;
}