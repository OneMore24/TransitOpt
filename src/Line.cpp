#include "Line.h"

CLine::CLine(int id, sf::Color color) : m_id(id), m_color(color) {}

int CLine::GetId() const { return m_id; }
sf::Color CLine::GetColor() const { return m_color; }
const std::vector<int>& CLine::GetStations() const { return m_stationIds; }

void CLine::AddStation(int stationId) {
    m_stationIds.push_back(stationId);
}

bool CLine::HasStation(int stationId) const {
    for (int id : m_stationIds) {
        if (id == stationId) return true;
    }
    return false;
}