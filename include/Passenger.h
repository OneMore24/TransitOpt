#ifndef __PASSENGER_H__
#define __PASSENGER_H__

#include <SFML/Graphics.hpp>
#include <vector>
#include "Station.h"

class CPassenger {
private:
    int m_id;
    int m_currentStationId; // donde esta parado aora
    StationType m_targetType;
    std::vector<int> m_path; // la ruta de estaciones q debe seguir
    size_t m_currentPathIndex; // en q paso de la ruta va
    sf::Vector2f m_pixelPos;
    bool m_onBoard; // si esta en un tren o esperando
    
public:
    CPassenger(int id, int start, StationType target, sf::Vector2f startPos);
    
    void SetPixelPos(sf::Vector2f pos) { m_pixelPos = pos; }
    sf::Vector2f GetPixelPos() const { return m_pixelPos; }
    
    int GetCurrentStationId() const { return m_currentStationId; }
    void SetCurrentStationId(int id) { m_currentStationId = id; }
    
    StationType GetTargetType() const { return m_targetType; }
    
    bool IsOnBoard() const { return m_onBoard; }
    void SetOnBoard(bool val) { m_onBoard = val; }

    // metodos pa la ruta y transbordos
    void SetPath(const std::vector<int>& newPath) { m_path = newPath; m_currentPathIndex = 0; }
    bool HasPath() const { return !m_path.empty(); }
    int GetNextStationId() const;
    void AdvancePath() { m_currentPathIndex++; }
    bool HasArrived() const { return m_currentPathIndex >= m_path.size() - 1; }
};

#endif