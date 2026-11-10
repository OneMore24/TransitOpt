#ifndef __GAME_H__
#define __GAME_H__

#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "Graph.h"
#include "Passenger.h"
#include "Line.h"
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
    
    int m_nextStationId;
    int m_nextPassengerId;
    int m_selectedStationId;
    int m_bridgesLeft;
    int m_draggingType;
    int m_lineaActualIndex;
    bool m_modoRio;
    
    sf::Clock m_clock;
    StationType m_tipos[10];

    void ProcessEvents();
    void Update(float deltaTime);

public:
    Game();
    void Run();
};

#endif