#include "Game.h"
#include "Renderer.h"
#include "GeneticAlgorithm.h"
#include <iostream>
#include <algorithm>

Game::Game() 
    : m_window(sf::VideoMode(sf::Vector2u(m_config.columns * m_config.cellSize + m_config.menuWidth, m_config.rows * m_config.cellSize)), "TransitOpt - Hito 8"),
      m_mapa(m_config.rows, m_config.columns, m_config.cellSize),
      m_redMetro(10),
      m_nextStationId(1), m_nextPassengerId(1), m_selectedStationId(-1),
      m_draggingType(-1), m_modoRio(false) {
    
    m_window.setFramerateLimit(60);

    // inicialisamos las 7 lineas con colores distintos
    // la IA las sobreescribira despues, pero necesitamos colores base
    m_lineas.emplace_back(1, sf::Color::Red);
    m_lineas.emplace_back(2, sf::Color::Blue);
    m_lineas.emplace_back(3, sf::Color::Green);
    m_lineas.emplace_back(4, sf::Color::Yellow);
    m_lineas.emplace_back(5, sf::Color::Magenta);
    m_lineas.emplace_back(6, sf::Color::Cyan);
    m_lineas.emplace_back(7, sf::Color(255, 165, 0)); // naranja

    m_tipos[0] = StationType::CIRCULO; m_tipos[1] = StationType::CUADRADO;
    m_tipos[2] = StationType::TRIANGULO; m_tipos[3] = StationType::OVALO;
    m_tipos[4] = StationType::PENTAGONO; m_tipos[5] = StationType::CONO;
    m_tipos[6] = StationType::DIAMANTE; m_tipos[7] = StationType::ESTRELLA;
    m_tipos[8] = StationType::RECTANGULO; m_tipos[9] = StationType::CRUZ;
}

void Game::Run() {
    while (m_window.isOpen()) {
        float deltaTime = m_clock.restart().asSeconds();
        ProcessEvents();
        Update(deltaTime);
        
        m_window.clear(sf::Color(240, 240, 240));
        
        // el orden de dibujo importa
        Renderer::DrawMap(m_window, m_mapa, m_config.menuWidth);
        Renderer::DrawLines(m_window, m_lineas, m_redMetro, m_mapa, m_config.menuWidth);
        Renderer::DrawStations(m_window, m_redMetro, m_selectedStationId, m_config.menuWidth, m_config.cellSize);
        Renderer::DrawPassengers(m_window, m_pasajeros, m_config.menuWidth);
        
        // el menu ahora solo muestra las figuras y el color de la linea activa (si se usa)
        Renderer::DrawMenu(m_window, m_tipos, m_config.menuWidth, m_modoRio, sf::Color::White);
        
        // dibujamos el fantasma si estamos arrastrando
        if (m_draggingType != -1) {
            sf::Vector2f mousePos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
            Renderer::DrawStationShape(m_window, static_cast<StationType>(m_draggingType), mousePos, m_config.cellSize / 3.0f, sf::Color(255, 255, 255, 200));
        }
        
        m_window.display();
    }
}

void Game::ProcessEvents() {
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) m_window.close();

        sf::Vector2f mousePos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));

        // teclas
        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::R) m_modoRio = !m_modoRio;
            if (key->code == sf::Keyboard::Key::G) RunAI(); // lansa la IA
        }

        // pintar rio arrastrando
        if (event->getIf<sf::Event::MouseMoved>()) {
            if (m_modoRio && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                if (mousePos.x >= m_config.menuWidth) {
                    sf::Vector2u gridCoords = m_mapa.GetGridCoords(mousePos.x - m_config.menuWidth, mousePos.y);
                    m_mapa.SetCell(gridCoords.x, gridCoords.y, TerrainType::AGUA);
                }
            }
        }

        if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButton->button == sf::Mouse::Button::Left) {
                // click en el menu pa arrastrar figura
                if (mousePos.x > 0 && mousePos.x < m_config.menuWidth) {
                    int index = mousePos.y / (m_window.getSize().y / 10.0f);
                    if (index >= 0 && index < 10) m_draggingType = static_cast<int>(m_tipos[index]);
                } 
                // click en el mapa
                else if (mousePos.x >= m_config.menuWidth) {
                    sf::Vector2u gridCoords = m_mapa.GetGridCoords(mousePos.x - m_config.menuWidth, mousePos.y);
                    if (m_modoRio) {
                        m_mapa.SetCell(gridCoords.x, gridCoords.y, TerrainType::AGUA);
                    } else {
                        // solo seleccionamos la estasion pa resaltarla, ya no unimos manual
                        auto* clickedStation = m_redMetro.FindStationAt(gridCoords.x, gridCoords.y);
                        if (clickedStation != nullptr) {
                            m_selectedStationId = clickedStation->GetId();
                        }
                    }
                }
            }

            // click derecho = pasajero
            if (mouseButton->button == sf::Mouse::Button::Right && mousePos.x >= m_config.menuWidth && !m_modoRio) {
                sf::Vector2u gridCoords = m_mapa.GetGridCoords(mousePos.x - m_config.menuWidth, mousePos.y);
                auto* clickedStation = m_redMetro.FindStationAt(gridCoords.x, gridCoords.y);
                if (clickedStation != nullptr && m_redMetro.GetNumStations() > 1) {
                    StationType targetType;
                    do { targetType = static_cast<StationType>(rand() % 10); } while (targetType == clickedStation->GetType());

                    std::vector<int> path = m_redMetro.FindShortestPathToType(clickedStation->GetId(), targetType);
                    if (!path.empty()) {
                        m_pasajeros.emplace_back(m_nextPassengerId++, clickedStation->GetId(), targetType, path, clickedStation->GetPixelPos());
                    }
                }
            }
        }

        // soltamos el mouse pa crear la estasion
        if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseButton->button == sf::Mouse::Button::Left && m_draggingType != -1) {
                if (mousePos.x >= m_config.menuWidth) {
                    sf::Vector2u gridCoords = m_mapa.GetGridCoords(mousePos.x - m_config.menuWidth, mousePos.y);
                    if (m_redMetro.FindStationAt(gridCoords.x, gridCoords.y) == nullptr) {
                        if (m_mapa.GetCell(gridCoords.x, gridCoords.y) == TerrainType::AGUA) {
                            std::cout << "no se puede poner estasion en el agua!\n";
                        } else {
                            sf::Vector2f pos((gridCoords.x * m_config.cellSize) + (m_config.cellSize / 2.0f), (gridCoords.y * m_config.cellSize) + (m_config.cellSize / 2.0f));
                            m_redMetro.AddStation(m_nextStationId++, gridCoords.x, gridCoords.y, pos, static_cast<StationType>(m_draggingType));
                        }
                    }
                }
                m_draggingType = -1;
            }
        }
    }
}

void Game::Update(float deltaTime) {
    for (auto& p : m_pasajeros) p.Update(deltaTime, &m_redMetro);
    m_pasajeros.erase(std::remove_if(m_pasajeros.begin(), m_pasajeros.end(), [](const CPassenger& p) { return p.HasArrived(); }), m_pasajeros.end());
}

void Game::RunAI() {
    std::vector<CStation<StationTrait<int>>*> stations;
    for (size_t i = 0; i < m_redMetro.GetNumStations(); ++i) stations.push_back(m_redMetro.GetStation(i));
    
    if (stations.size() < 2) {
        std::cout << "pone mas estaciones!\n";
        return;
    }

    // generamos demandas (simulamos pasajeros q quieren viajar)
    std::vector<std::pair<int, StationType>> demands;
    for (auto* st : stations) {
        for (int k = 0; k < 5; ++k) {
            StationType t;
            do { t = static_cast<StationType>(rand() % 10); } while (t == st->GetType());
            demands.push_back({st->GetId(), t});
        }
    }

    // Llamamos a la IA con el maximo de lineas y puentes definidos en config
    GeneticAlgorithm ga(stations, m_mapa, m_config.maxBridges, m_config.maxLines, demands, 50, 50);
    std::vector<CLine> bestLines = ga.Optimize();

    if (bestLines.empty()) {
        std::cout << "la IA no encontro una solucion valida\n";
        return;
    }

        // Limpiamos las vias y lineas actuales
    m_redMetro.ClearEdges();
    m_lineas = bestLines;

        // Asignamos colores y reasignamos IDs a las lineas que creo la IA
    sf::Color colores[7] = {sf::Color::Red, sf::Color::Blue, sf::Color::Green, sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan, sf::Color(255, 165, 0)};
    for (size_t i = 0; i < m_lineas.size(); ++i) {
        // 1. creamos la nueva linea con su color
        CLine lineaConColor(i + 1, colores[i % 7]);
        
        // 2. le copiamos las estaciones de la linea original
        for (int stId : m_lineas[i].GetStations()) {
            lineaConColor.AddStation(stId);
        }
        
        // 3. la reemplasamos
        m_lineas[i] = lineaConColor;
        
        const auto& stIds = m_lineas[i].GetStations();
        if (stIds.size() < 2) continue; // proteccion por si la IA devolvio una linea vacia

        // 4. reconstruimos el grafo
        for (size_t j = 0; j < stIds.size() - 1; ++j) {
            auto* stA = m_redMetro.GetStationById(stIds[j]);
            auto* stB = m_redMetro.GetStationById(stIds[j+1]);
            if (stA && stB) {
                bool cruza = m_mapa.IsRiverCrossing(stA->GetPixelPos().x, stA->GetPixelPos().y, stB->GetPixelPos().x, stB->GetPixelPos().y);
                m_redMetro.AddEdge(stIds[j], stIds[j+1], cruza);
            }
        }
    }

    std::cout << "====================================\n";
    std::cout << "IA aplicada al mapa!\n";
    std::cout << "Rutas creadas: " << m_lineas.size() << "\n";
    std::cout << "====================================\n";
    
    m_window.setTitle("TransitOpt - IA aplicada | Rutas: " + std::to_string(m_lineas.size()));
}