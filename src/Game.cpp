#include "Game.h"
#include "Renderer.h"
#include "GeneticAlgorithm.h"
#include "Train.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <random> // <--- Añadido pa el shuffle
#include "json.hpp"

using json = nlohmann::json;

Game::Game() 
    : m_window(sf::VideoMode(sf::Vector2u(m_config.columns * m_config.cellSize + m_config.menuWidth, m_config.rows * m_config.cellSize)), "TransitOpt - Hito 12"),
      m_mapa(m_config.rows, m_config.columns, m_config.cellSize),
      m_redMetro(10),
      m_nextStationId(1), m_nextPassengerId(1), m_selectedStationId(-1),
      m_draggingType(-1), m_modoRio(false), m_pausado(false),
      m_totalPassengers(0), m_arrivedPassengers(0), m_trainCapacity(10),
      m_isOptimizing(false), m_currentTrackIndex(0), m_musicPlaying(false) {
    
    m_window.setFramerateLimit(60);

    if (!m_font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cout << "no se pudo cargar la fuente, el HUD no se vera\n";
    }

    LoadConfig();      
    SetupConfig();     

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

void Game::LoadConfig() {
    std::ifstream f("config.json");
    if (f.is_open()) {
        json j;
        f >> j;
        if (j.contains("mapa")) m_config.maxBridges = j["mapa"].value("max_puentes", 3);
        if (j.contains("simulacion")) {
            m_config.maxLines = j["simulacion"].value("max_lineas", 7);
            m_config.autoSpawn = j["simulacion"].value("autopiloto", false);
            m_config.spawnInterval = j["simulacion"].value("intervalo_spawn", 2.0f);
            m_config.gaPopulation = j["simulacion"].value("ia_redes", 1000);
            m_config.gaGenerations = j["simulacion"].value("ia_generaciones", 500);
            
            if (j["simulacion"].contains("playlist")) {
                for (auto& track : j["simulacion"]["playlist"]) {
                    m_config.playlist.push_back(track.get<std::string>());
                }
            }
        }
        if (j.contains("flota")) {
            m_trainCapacity = j["flota"].value("capacidad_vagon", 10);
            m_config.trainSpeed = j["flota"].value("velocidad_base", 100.0f);
        }
    }
}

void Game::SetupConfig() {
    std::cout << "\n=========================================\n";
    std::cout << "   CONFIGURACION DE TRANSIT-OPT\n";
    std::cout << "=========================================\n";
    std::cout << "presiona enter pa usar el valor por defecto\n\n";

    std::string input;

    std::cout << "cuantas rutas maximas quieres crear? [" << m_config.maxLines << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) { try { m_config.maxLines = std::stoi(input); } catch (...) {} }

    std::cout << "cuantos puentes maximos tienes? [" << m_config.maxBridges << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) { try { m_config.maxBridges = std::stoi(input); } catch (...) {} }

    std::cout << "cuantos pasajeros caben en un tren? [" << m_trainCapacity << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) { try { m_trainCapacity = std::stoi(input); } catch (...) {} }

    std::cout << "cuantas redes iniciales pa la IA? [" << m_config.gaPopulation << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) { try { m_config.gaPopulation = std::stoi(input); } catch (...) {} }

    std::cout << "cuantas generaciones pa la IA? [" << m_config.gaGenerations << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) { try { m_config.gaGenerations = std::stoi(input); } catch (...) {} }

    std::cout << "activar autopiloto de pasajeros? (s/n) [";
    std::cout << (m_config.autoSpawn ? "s" : "n") << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) { m_config.autoSpawn = (input == "s" || input == "S"); }

    if (m_config.autoSpawn) {
        std::cout << "cada cuantos segundos aparece un pasajero por estacion? [" << m_config.spawnInterval << "]: ";
        std::getline(std::cin, input);
        if (!input.empty()) { try { m_config.spawnInterval = std::stof(input); } catch (...) {} }
    }

    std::cout << "\nConfiguracion lista! Abriendo el simulador...\n\n";
}

void Game::ClearMap() {
    m_redMetro = CGraph<StationTrait<int>>(10);
    m_pasajeros.clear();
    m_trenes.clear();
    m_lineas.clear();
    
    m_lineas.emplace_back(1, sf::Color::Red);
    m_lineas.emplace_back(2, sf::Color::Blue);
    m_lineas.emplace_back(3, sf::Color::Green);
    m_lineas.emplace_back(4, sf::Color::Yellow);
    m_lineas.emplace_back(5, sf::Color::Magenta);
    m_lineas.emplace_back(6, sf::Color::Cyan);
    m_lineas.emplace_back(7, sf::Color(255, 165, 0));

    m_nextStationId = 1;
    m_nextPassengerId = 1;
    m_selectedStationId = -1;
    m_totalPassengers = 0;
    m_arrivedPassengers = 0;
    m_window.setTitle("TransitOpt - Hito 12 | Rutas: 0");
}

void Game::Run() {
    while (m_window.isOpen()) {
        float deltaTime = m_clock.restart().asSeconds();
        ProcessEvents();
        Update(deltaTime);
        
        m_window.clear(sf::Color(240, 240, 240));
        
        Renderer::DrawMap(m_window, m_mapa, m_config.menuWidth);
        Renderer::DrawLines(m_window, m_lineas, m_redMetro, m_mapa, m_config.menuWidth);
        
        for (auto& tren : m_trenes) {
            tren.Draw(m_window, m_config.menuWidth);
            sf::Text capText(m_font);
            capText.setCharacterSize(10);
            capText.setFillColor(sf::Color::White);
            capText.setString(std::to_string(tren.GetPassengers()) + "/" + std::to_string(tren.GetCapacity()));
            capText.setPosition(sf::Vector2f(tren.GetPixelPos().x + m_config.menuWidth - 8, tren.GetPixelPos().y - 20));
            m_window.draw(capText);
        }
        
        Renderer::DrawStations(m_window, m_redMetro, m_selectedStationId, m_config.menuWidth, m_config.cellSize);
        Renderer::DrawPassengers(m_window, m_pasajeros, m_config.menuWidth);
        Renderer::DrawMenu(m_window, m_tipos, m_config.menuWidth, m_modoRio, sf::Color::White);
        
        if (m_draggingType != -1) {
            sf::Vector2f mousePos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
            Renderer::DrawStationShape(m_window, static_cast<StationType>(m_draggingType), mousePos, m_config.cellSize / 3.0f, sf::Color(255, 255, 255, 200));
        }

        Renderer::DrawHUD(m_window, m_font, m_lineas.size(), m_totalPassengers, m_arrivedPassengers, m_trainCapacity, m_pausado);
        
        if (m_isOptimizing) {
            sf::RectangleShape overlay(sf::Vector2f(m_window.getSize().x, m_window.getSize().y));
            overlay.setFillColor(sf::Color(0, 0, 0, 180));
            m_window.draw(overlay);
            
            sf::Text txt(m_font);
            txt.setString("LA IA ESTA CALCULANDO LA RED OPTIMA...\nPOR FAVOR ESPERA (esto puede tardar varios minutos)");
            txt.setCharacterSize(24);
            txt.setFillColor(sf::Color::White);
            txt.setPosition(sf::Vector2f(200, m_window.getSize().y / 2.0f - 50));
            m_window.draw(txt);
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
            if (key->code == sf::Keyboard::Key::P) m_pausado = !m_pausado;
            if (key->code == sf::Keyboard::Key::C) ClearMap();
            if (key->code == sf::Keyboard::Key::A) {
                m_config.autoSpawn = !m_config.autoSpawn;
                std::cout << "autopiloto: " << (m_config.autoSpawn ? "ON" : "OFF") << "\n";
            }
            if (key->code == sf::Keyboard::Key::S) SaveMap();
            if (key->code == sf::Keyboard::Key::L) LoadMap();
            
            if (key->code == sf::Keyboard::Key::Add) m_trainCapacity++;
            if (key->code == sf::Keyboard::Key::Subtract) {
                if (m_trainCapacity > 1) m_trainCapacity--;
            }
        }

        if (!m_pausado) {
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
                            if (clickedStation != nullptr) m_selectedStationId = clickedStation->GetId();
                        }
                    }
                }

                if (mouseButton->button == sf::Mouse::Button::Right && mousePos.x >= m_config.menuWidth && !m_modoRio) {
                    sf::Vector2u gridCoords = m_mapa.GetGridCoords(mousePos.x - m_config.menuWidth, mousePos.y);
                    auto* clickedStation = m_redMetro.FindStationAt(gridCoords.x, gridCoords.y);
                    if (clickedStation != nullptr && m_redMetro.GetNumStations() > 1) {
                        StationType targetType;
                        do { targetType = static_cast<StationType>(rand() % 10); } while (targetType == clickedStation->GetType());
                        m_pasajeros.emplace_back(m_nextPassengerId++, clickedStation->GetId(), targetType, clickedStation->GetPixelPos());
                        m_totalPassengers++;
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
}

void Game::Update(float deltaTime) {
    if (m_pausado) return;

    if (m_musicPlaying && m_music.getStatus() == sf::Music::Status::Stopped) {
        PlayNextTrack();
    }

    if (m_isOptimizing && m_optimizeFuture.valid()) {
        if (m_optimizeFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            FinishAIOptimization();
        }
    }

    if (m_config.autoSpawn && m_spawnClock.getElapsedTime().asSeconds() > m_config.spawnInterval) {
        for (size_t i = 0; i < m_redMetro.GetNumStations(); ++i) {
            auto* st = m_redMetro.GetStation(i);
            if (st) {
                StationType targetType;
                do { targetType = static_cast<StationType>(rand() % 10); } while (targetType == st->GetType());
                m_pasajeros.emplace_back(m_nextPassengerId++, st->GetId(), targetType, st->GetPixelPos());
                m_totalPassengers++;
            }
        }
        m_spawnClock.restart();
    }

    for (auto& tren : m_trenes) tren.Update(deltaTime, m_redMetro);

    for (auto it = m_pasajeros.begin(); it != m_pasajeros.end(); ) {
        bool deletePassenger = false;

        if (!it->IsOnBoard()) {
            if (!it->HasPath() || it->HasArrived()) {
                std::vector<int> path = m_redMetro.FindShortestPathToType(it->GetCurrentStationId(), it->GetTargetType());
                if (!path.empty()) it->SetPath(path);
            }

            int nextStationId = it->GetNextStationId();
            if (nextStationId != -1) {
                for (auto& tren : m_trenes) {
                    if (tren.GetState() == TrainState::WAITING && tren.HasSpace()) {
                        int trainStationId = tren.GetCurrentStationId(m_redMetro);
                        if (trainStationId == it->GetCurrentStationId()) {
                            const auto& stIds = m_lineas[tren.GetLineId()-1].GetStations();
                            bool trainGoesToNext = false;
                            for (int stId : stIds) {
                                if (stId == nextStationId) { trainGoesToNext = true; break; }
                            }

                            if (trainGoesToNext) {
                                tren.BoardPassenger();
                                it->SetOnBoard(true);
                                it->SetPixelPos(tren.GetPixelPos());
                                break; 
                            }
                        }
                    }
                }
            }
        } else {
            for (auto& tren : m_trenes) {
                if (tren.GetState() == TrainState::WAITING) {
                    int trainStationId = tren.GetCurrentStationId(m_redMetro);
                    auto* currentSt = m_redMetro.GetStationById(trainStationId);
                    
                    if (currentSt) {
                        if (currentSt->GetType() == it->GetTargetType()) {
                            tren.UnboardPassenger();
                            deletePassenger = true;
                            m_arrivedPassengers++;
                            break;
                        }
                        
                        if (trainStationId == it->GetNextStationId()) {
                            tren.UnboardPassenger();
                            it->SetOnBoard(false);
                            it->SetCurrentStationId(trainStationId);
                            it->AdvancePath();
                            it->SetPixelPos(currentSt->GetPixelPos());
                            break;
                        }
                    }
                    it->SetPixelPos(tren.GetPixelPos());
                }
            }
        }

        if (deletePassenger) it = m_pasajeros.erase(it);
        else ++it;
    }
}

void Game::SaveMap() {
    json j;
    j["stations"] = json::array();
    for (size_t i = 0; i < m_redMetro.GetNumStations(); ++i) {
        auto* st = m_redMetro.GetStation(i);
        j["stations"].push_back({
            {"id", st->GetId()},
            {"x", st->GetGridX()},
            {"y", st->GetGridY()},
            {"type", static_cast<int>(st->GetType())}
        });
    }

    j["lines"] = json::array();
    for (auto& line : m_lineas) {
        json lineJson;
        lineJson["id"] = line.GetId();
        lineJson["color"] = {line.GetColor().r, line.GetColor().g, line.GetColor().b};
        lineJson["stations"] = line.GetStations();
        j["lines"].push_back(lineJson);
    }

    std::ofstream f("mapa_guardado.json");
    f << j.dump(4);
    f.close();
    std::cout << "Mapa guardado en mapa_guardado.json\n";
}

void Game::LoadMap() {
    std::ifstream f("mapa_guardado.json");
    if (!f.is_open()) {
        std::cout << "no hay mapa guardado\n";
        return;
    }
    
    json j;
    f >> j;
    ClearMap();

    for (auto& st : j["stations"]) {
        sf::Vector2f pos((st["x"].get<int>() * m_config.cellSize) + (m_config.cellSize / 2.0f), 
                         (st["y"].get<int>() * m_config.cellSize) + (m_config.cellSize / 2.0f));
        m_redMetro.AddStation(st["id"], st["x"], st["y"], pos, static_cast<StationType>(st["type"].get<int>()));
        if (st["id"].get<int>() >= m_nextStationId) m_nextStationId = st["id"].get<int>() + 1;
    }

    for (auto& lineJson : j["lines"]) {
        CLine line(lineJson["id"], sf::Color(lineJson["color"][0], lineJson["color"][1], lineJson["color"][2]));
        for (int stId : lineJson["stations"]) line.AddStation(stId);
        m_lineas.push_back(line);
        
        const auto& stIds = line.GetStations();
        for (size_t i = 0; i < stIds.size() - 1; ++i) {
            auto* stA = m_redMetro.GetStationById(stIds[i]);
            auto* stB = m_redMetro.GetStationById(stIds[i+1]);
            if (stA && stB) {
                bool cruza = m_mapa.IsRiverCrossing(stA->GetPixelPos().x, stA->GetPixelPos().y, stB->GetPixelPos().x, stB->GetPixelPos().y);
                m_redMetro.AddEdge(stIds[i], stIds[i+1], cruza);
            }
        }
    }

    CreateTrainsForLines();
    std::cout << "Mapa cargado con exito!\n";
}

void Game::RunAI() {
    if (m_isOptimizing) return;
    if (m_redMetro.GetNumStations() < 2) {
        std::cout << "pone mas estaciones!\n";
        return;
    }

    m_isOptimizing = true;
    std::cout << "lanzando IA en segundo plano...\n";

    // MUSICA: mezclamos la lista pa q sea aleatoria
    if (!m_config.playlist.empty()) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(m_config.playlist.begin(), m_config.playlist.end(), g);

        m_currentTrackIndex = 0;
        if (m_music.openFromFile(m_config.playlist[0])) {
            m_music.play();
            m_musicPlaying = true;
            std::cout << "reproduciendo: " << m_config.playlist[0] << "\n";
        } else {
            std::cout << "no se pudo cargar la musica: " << m_config.playlist[0] << "\n";
        }
    }

    m_optimizeFuture = std::async(std::launch::async, [this]() {
        std::vector<CStation<StationTrait<int>>*> stations;
        for (size_t i = 0; i < m_redMetro.GetNumStations(); ++i) stations.push_back(m_redMetro.GetStation(i));
        
        std::vector<std::pair<int, StationType>> demands;
        for (auto* st : stations) {
            for (int k = 0; k < 5; ++k) {
                StationType t;
                do { t = static_cast<StationType>(rand() % 10); } while (t == st->GetType());
                demands.push_back({st->GetId(), t});
            }
        }

        GeneticAlgorithm ga(stations, m_mapa, m_config.maxBridges, m_config.maxLines, demands, m_config.gaPopulation, m_config.gaGenerations);
        return ga.Optimize();
    });
}

void Game::FinishAIOptimization() {
    std::vector<CLine> bestLines = m_optimizeFuture.get();
    m_isOptimizing = false;

    if (m_musicPlaying) {
        m_music.stop();
        m_musicPlaying = false;
        std::cout << "musica detenida\n";
    }

    if (bestLines.empty()) { std::cout << "la IA no encontro solucion\n"; return; }

    m_redMetro.ClearEdges();
    m_lineas = bestLines;

    sf::Color colores[7] = {sf::Color::Red, sf::Color::Blue, sf::Color::Green, sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan, sf::Color(255, 165, 0)};
    for (size_t i = 0; i < m_lineas.size(); ++i) {
        CLine lineaConColor(i + 1, colores[i % 7]);
        for (int stId : m_lineas[i].GetStations()) lineaConColor.AddStation(stId);
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

    CreateTrainsForLines();
    m_window.setTitle("TransitOpt - IA aplicada | Rutas: " + std::to_string(m_lineas.size()));
    std::cout << "IA aplicada al mapa!\n";
}

void Game::CreateTrainsForLines() {
    m_trenes.clear();
    for (auto& line : m_lineas) {
        if (line.GetStations().size() >= 2) {
            m_trenes.emplace_back(line.GetId(), &line, 0, m_trainCapacity); 
        }
    }
}

void Game::PlayNextTrack() {
    if (m_config.playlist.empty()) return;
    
    m_currentTrackIndex++;
    if (static_cast<size_t>(m_currentTrackIndex) >= m_config.playlist.size()) {
        m_currentTrackIndex = 0;
        
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(m_config.playlist.begin(), m_config.playlist.end(), g);
    }

    std::string track = m_config.playlist[m_currentTrackIndex];
    if (m_music.openFromFile(track)) {
        m_music.play();
        std::cout << "reproduciendo: " << track << "\n";
    } else {
        std::cout << "no se pudo cargar la cancion: " << track << "\n";
    }
}