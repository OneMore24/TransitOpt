#include "Passenger.h"

CPassenger::CPassenger(int id, int start, StationType target, sf::Vector2f startPos)
    : m_id(id), m_currentStationId(start), m_targetType(target), 
      m_currentPathIndex(0), m_pixelPos(startPos), m_onBoard(false) {}

int CPassenger::GetNextStationId() const {
    if (m_path.empty() || m_currentPathIndex >= m_path.size() - 1) return -1;
    return m_path[m_currentPathIndex + 1];
}