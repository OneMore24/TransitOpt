#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "Graph.h"
#include "Line.h"
#include "Config.h"
#include <vector>

class Renderer {
public:
    static void DrawStationShape(sf::RenderWindow& window, StationType type, sf::Vector2f pos, float size, sf::Color color);
    static void DrawMap(sf::RenderWindow& window, const CGrid<GridTrait<TerrainType>>& mapa, float offsetX);
    static void DrawLines(sf::RenderWindow& window, const std::vector<CLine>& lineas, const CGraph<StationTrait<int>>& redMetro, const CGrid<GridTrait<TerrainType>>& mapa, float offsetX);
    static void DrawStations(sf::RenderWindow& window, const CGraph<StationTrait<int>>& redMetro, int selectedId, float offsetX, int cellSize);
    static void DrawMenu(sf::RenderWindow& window, const StationType tipos[10], int menuWidth, bool modoRio, sf::Color lineaActualColor);
    
    // HUD simplificado (solo muestra rutas y si la IA esta trabajando)
    static void DrawHUD(sf::RenderWindow& window, const sf::Font& font, int rutas, bool optimizando);
};

#endif