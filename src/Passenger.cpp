#include "Passenger.h"

CPassenger::CPassenger(int id, int start, StationType target, sf::Vector2f startPos)
    : m_id(id), m_startId(start), m_targetType(target), m_pixelPos(startPos), m_onBoard(false) {
    // constructor limpio, ya no creamos la figura aqui
}