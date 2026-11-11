#ifndef __GAME_H__
#define __GAME_H__

#include <SFML/Graphics.hpp>
#include <future>
#include "Grid.h"
#include "Graph.h"
#include "Passenger.h"
#include "Line.h"
#include "Train.h"
#include "Config.h"
#include <vector>

class Game {
private:
    GameConfig m_config;
    sf::RenderWindow m_window;
    CGrid<GridTrait<TerrainType>> m_mapa;
    CGraph<StationTrait<int>> m_redMetro;
    
    std::vector<CPassenger> m_pasajeros;
    std::vector<CLine> m_lineas;
    std::vector<CTrain> m_trenes;
    
    int m_nextStationId;
    int m_nextPassengerId;
    int m_selectedStationId;
    int m_draggingType;
    bool m_modoRio;
    bool m_pausado;
    
    int m_totalPassengers;
    int m_arrivedPassengers;
    int m_trainCapacity;
    
    sf::Clock m_clock;
    sf::Clock m_spawnClock;
    StationType m_tipos[10];
    sf::Font m_font; 

    // variables pa la concurrencia de la IA
    bool m_isOptimizing;
    std::future<std::vector<CLine>> m_optimizeFuture;

    void ProcessEvents();
    void Update(float deltaTime);
    void RunAI();
    void FinishAIOptimization();
    void CreateTrainsForLines();
    void LoadConfig();
    void SetupConfig();
    void ClearMap();
    void SaveMap();
    void LoadMap();

public:
    Game();
    void Run();
};

#endif