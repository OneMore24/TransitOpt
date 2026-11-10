#include "Game.h"
#include "Renderer.h"
#include "GeneticAlgorithm.h"
#include "Train.h"
#include <iostream>
#include <algorithm>
#include <cmath>

Game::Game() 
    : m_window(sf::VideoMode(sf::Vector2u(m_config.columns * m_config.cellSize + m_config.menuWidth, m_config.rows * m_config.cellSize)), "TransitOpt - Hito 9"),
      m_mapa(m_config.rows, m_config.columns, m_config.cellSize),
      m_redMetro(10),
      m_nextStationId(1), m_nextPassengerId(1), m_selectedStationId(-1),
      m_draggingType(-1), m_modoRio(false) {
    
    m_window.setFramerateLimit(60);

    m_lineas.emplace_back(1, sf::Color::Red);
    m_lineas.emplace_back(2, sf::Color::Blue);
    m_lineas.emplace_back(3, sf::Color::Green);
    m_lineas.emplace_back(4, sf::Color::Yellow);
    m_lineas.emplace_back(5, sf::Color::Magenta);
    m_lineas.emplace_back(6, sf::Color::Cyan);
    m_lineas.emplace_back(7, sf::Color(255, 165, 0));

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
        
        // orden de dibujo
        Renderer::DrawMap(m_window, m_mapa, m_config.menuWidth);
        Renderer::DrawLines(m_window, m_lineas, m_redMetro, m_mapa, m_config.menuWidth);
        
        // dibujamos los trenes ANTES de las estaciones pa q no las tapen
        for (auto& tren : m_trenes) {
            tren.Draw(m_window, m_config.menuWidth);
        }
        
        Renderer::DrawStations(m_window, m_redMetro, m_selectedStationId, m_config.menuWidth, m_config.cellSize);
        Renderer::DrawPassengers(m_window, m_pasajeros, m_config.menuWidth);
        Renderer::DrawMenu(m_window, m_tipos, m_config.menuWidth, m_modoRio, sf::Color::White);
        
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

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::R) m_modoRio = !m_modoRio;
            if (key->code == sf::Keyboard::Key::G) RunAI();
        }

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
                if (mousePos.x > 0 && mousePos.x < m_config.menuWidth) {
                    int index = mousePos.y / (m_window.getSize().y / 10.0f);
                    if (index >= 0 && index < 10) m_draggingType = static_cast<int>(m_tipos[index]);
                } 
                else if (mousePos.x >= m_config.menuWidth) {
                    sf::Vector2u gridCoords = m_mapa.GetGridCoords(mousePos.x - m_config.menuWidth, mousePos.y);
                    if (m_modoRio) {
                        m_mapa.SetCell(gridCoords.x, gridCoords.y, TerrainType::AGUA);
                    } else {
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

                    // El pasajero ya no se mueve solo, asi q no le pasamos la ruta
                    m_pasajeros.emplace_back(m_nextPassengerId++, clickedStation->GetId(), targetType, clickedStation->GetPixelPos());
                }
            }
        }

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
    // 1. actualisamos trenes
    for (auto& tren : m_trenes) {
        tren.Update(deltaTime, m_redMetro);
    }

    // 2. logica de pasajeros (esperar o subirse)
    for (auto& pasajero : m_pasajeros) {
        if (!pasajero.IsOnBoard()) {
            // si esta esperando, buscamos un tren en su estasion
            for (auto& tren : m_trenes) {
                if (tren.GetState() == TrainState::WAITING && tren.HasSpace()) {
                    auto* stPasajero = m_redMetro.GetStationById(pasajero.GetStartId());
                    if (stPasajero && stPasajero->GetPixelPos() == tren.GetPixelPos()) {
                        tren.BoardPassenger();
                        pasajero.SetOnBoard(true);
                        
                        // ACTUALISAMOS LA POSISION DEL PASAJERO AL TREN
                        pasajero.SetPixelPos(tren.GetPixelPos());
                        break;
                    }
                }
            }
        } else {
            // si esta en el tren, su posision es la del tren
            // buscamos el tren en el q esta (simplificasion: el primero q lo tenga)
            // En un sistema real, el pasajero guardaria el ID del tren.
            // Pa esta version, lo hacemos rebotar con el primer tren q este cerca.
            for (auto& tren : m_trenes) {
                if (tren.GetState() == TrainState::MOVING) {
                    // si el tren esta serca del pasajero, lo movemos con el
                    float dx = pasajero.GetPixelPos().x - tren.GetPixelPos().x;
                    float dy = pasajero.GetPixelPos().y - tren.GetPixelPos().y;
                    if (std::sqrt(dx*dx + dy*dy) < 30.0f) {
                        pasajero.SetPixelPos(tren.GetPixelPos());
                        break;
                    }
                }
            }
        }
    }
}

void Game::RunAI() {
    std::vector<CStation<StationTrait<int>>*> stations;
    for (size_t i = 0; i < m_redMetro.GetNumStations(); ++i) stations.push_back(m_redMetro.GetStation(i));
    
    if (stations.size() < 2) {
        std::cout << "pone mas estaciones!\n";
        return;
    }

    std::vector<std::pair<int, StationType>> demands;
    for (auto* st : stations) {
        for (int k = 0; k < 5; ++k) {
            StationType t;
            do { t = static_cast<StationType>(rand() % 10); } while (t == st->GetType());
            demands.push_back({st->GetId(), t});
        }
    }

    GeneticAlgorithm ga(stations, m_mapa, m_config.maxBridges, m_config.maxLines, demands, 50, 50);
    std::vector<CLine> bestLines = ga.Optimize();

    if (bestLines.empty()) {
        std::cout << "la IA no encontro una solucion valida\n";
        return;
    }

    m_redMetro.ClearEdges();
    m_lineas = bestLines;

    sf::Color colores[7] = {sf::Color::Red, sf::Color::Blue, sf::Color::Green, sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan, sf::Color(255, 165, 0)};
    for (size_t i = 0; i < m_lineas.size(); ++i) {
        CLine lineaConColor(i + 1, colores[i % 7]);
        for (int stId : m_lineas[i].GetStations()) {
            lineaConColor.AddStation(stId);
        }
        m_lineas[i] = lineaConColor;
        
        const auto& stIds = m_lineas[i].GetStations();
        if (stIds.size() < 2) continue;

        for (size_t j = 0; j < stIds.size() - 1; ++j) {
            auto* stA = m_redMetro.GetStationById(stIds[j]);
            auto* stB = m_redMetro.GetStationById(stIds[j+1]);
            if (stA && stB) {
                bool cruza = m_mapa.IsRiverCrossing(stA->GetPixelPos().x, stA->GetPixelPos().y, stB->GetPixelPos().x, stB->GetPixelPos().y);
                m_redMetro.AddEdge(stIds[j], stIds[j+1], cruza);
            }
        }
    }

    // creamos los trenes pa las lineas nuevas
    CreateTrainsForLines();

    std::cout << "====================================\n";
    std::cout << "IA aplicada al mapa!\n";
    std::cout << "Rutas creadas: " << m_lineas.size() << "\n";
    std::cout << "====================================\n";
    
    m_window.setTitle("TransitOpt - IA aplicada | Rutas: " + std::to_string(m_lineas.size()));
}

void Game::CreateTrainsForLines() {
    m_trenes.clear();
    for (auto& line : m_lineas) {
        if (line.GetStations().size() >= 2) {
            // creamos un tren con capacidad 10 en la primera estasion de la linea
            m_trenes.emplace_back(line.GetId(), &line, 0, 10); 
        }
    }
}