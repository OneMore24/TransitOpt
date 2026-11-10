#ifndef __PASSENGER_H__
#define __PASSENGER_H__

#include <SFML/Graphics.hpp>
#include "Station.h"

class CPassenger {
private:
    int m_id;
    int m_startId;
    StationType m_targetType;
    sf::Vector2f m_pixelPos;
    bool m_onBoard; // si esta en un tren o esperando en el anden

public:
    CPassenger(int id, int start, StationType target, sf::Vector2f startPos);
    
    // El pasajero ya no se mueve solo, su posision la dicta el tren
    void SetPixelPos(sf::Vector2f pos) { m_pixelPos = pos; }
    sf::Vector2f GetPixelPos() const { return m_pixelPos; }
    
    int GetStartId() const { return m_startId; }
    StationType GetTargetType() const { return m_targetType; }
    
    bool IsOnBoard() const { return m_onBoard; }
    void SetOnBoard(bool val) { m_onBoard = val; }
};

#endif