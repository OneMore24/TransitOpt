#ifndef __GAME_H__
#define __GAME_H__

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <future>
#include "Grid.h"
#include "Graph.h"
#include "Line.h"
#include "Config.h"
#include <vector>

class Game {
private:
    GameConfig m_config;
    sf::RenderWindow m_window;
    CGrid<GridTrait<TerrainType>> m_mapa;
    CGraph<StationTrait<int>> m_redMetro;
    
    std::vector<CLine> m_lineas;
    
    int m_nextStationId;
    int m_selectedStationId;
    int m_draggingType;
    bool m_modoRio;
    bool m_pausado;
    
    sf::Clock m_clock;
    StationType m_tipos[10];
    sf::Font m_font; 

    bool m_isOptimizing;
    std::future<std::vector<CLine>> m_optimizeFuture;

    sf::Music m_music;
    int m_currentTrackIndex;
    bool m_musicPlaying;

    void ProcessEvents();
    void RunAI();
    void FinishAIOptimization();
    void LoadConfig();
    void SetupConfig();
    void ClearMap();
    void SaveMap();
    void LoadMap();
    void PlayNextTrack();

public:
    Game();
    void Run();
};

#endif