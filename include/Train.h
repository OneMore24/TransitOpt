#ifndef __TRAIN_H__
#define __TRAIN_H__

#include <SFML/Graphics.hpp>
#include <vector>
#include "Line.h"
#include "Graph.h"
#include "Station.h" // <--- Añadido pa q el editor no se queje

enum class TrainState {
    MOVING,
    WAITING
};

class CTrain {
private:
    int m_lineId;
    CLine* m_line;
    int m_currentStationIndex;
    int m_direction; 
    int m_capacity;
    int m_passengersOnBoard;
    
    sf::Vector2f m_pixelPos;
    float m_speed;
    TrainState m_state;
    float m_waitTimer;

public:
    CTrain(int lineId, CLine* line, int startIndex, int capacity);
    
    void Update(float deltaTime, const CGraph<StationTrait<int>>& graph);
    void Draw(sf::RenderWindow& window, float offsetX);
    
    int GetLineId() const { return m_lineId; }
    int GetCapacity() const { return m_capacity; }
    int GetPassengers() const { return m_passengersOnBoard; }
    bool HasSpace() const { return m_passengersOnBoard < m_capacity; }
    void BoardPassenger() { m_passengersOnBoard++; }
    void UnboardPassenger() { if (m_passengersOnBoard > 0) m_passengersOnBoard--; }
    
    sf::Vector2f GetPixelPos() const { return m_pixelPos; }
    int GetCurrentStationIndex() const { return m_currentStationIndex; }
    TrainState GetState() const { return m_state; }
    
    int GetCurrentStationId(const CGraph<StationTrait<int>>& graph) const;
};

#endif