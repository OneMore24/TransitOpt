#include "Train.h"
#include <cmath>

CTrain::CTrain(int lineId, CLine* line, int startIndex, int capacity)
    : m_lineId(lineId), m_line(line), m_currentStationIndex(startIndex), 
      m_direction(1), m_capacity(capacity), m_passengersOnBoard(0), 
      m_speed(100.0f), m_state(TrainState::WAITING), m_waitTimer(1.0f) {
}

void CTrain::Update(float deltaTime, const CGraph<StationTrait<int>>& graph) {
    const auto& stIds = m_line->GetStations();
    if (stIds.size() < 2) return;

    auto* currentSt = graph.GetStationById(stIds[m_currentStationIndex]);
    if (!currentSt) return;
    
    if (m_state == TrainState::WAITING) {
        m_pixelPos = currentSt->GetPixelPos();
        m_waitTimer -= deltaTime;
        if (m_waitTimer <= 0.0f) {
            m_state = TrainState::MOVING;
            m_waitTimer = 1.0f;
        }
    } 
    else if (m_state == TrainState::MOVING) {
        int nextIndex = m_currentStationIndex + m_direction;
        if (nextIndex >= static_cast<int>(stIds.size()) || nextIndex < 0) {
            m_direction *= -1;
            nextIndex = m_currentStationIndex + m_direction;
        }

        auto* nextSt = graph.GetStationById(stIds[nextIndex]);
        if (!nextSt) return;

        sf::Vector2f targetPos = nextSt->GetPixelPos();
        sf::Vector2f dir = targetPos - m_pixelPos;
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

        if (dist < 5.0f) {
            m_currentStationIndex = nextIndex;
            m_pixelPos = targetPos;
            m_state = TrainState::WAITING; 
            m_waitTimer = 1.0f;
        } else {
            dir /= dist;
            m_pixelPos += dir * m_speed * deltaTime;
        }
    }
}

void CTrain::Draw(sf::RenderWindow& window, float offsetX) {
    sf::RectangleShape rect(sf::Vector2f(16, 10));
    rect.setOrigin(sf::Vector2f(8, 5));
    rect.setPosition(sf::Vector2f(m_pixelPos.x + offsetX, m_pixelPos.y));
    rect.setFillColor(m_line->GetColor());
    rect.setOutlineThickness(2);
    rect.setOutlineColor(sf::Color::Black);
    window.draw(rect);
}

int CTrain::GetCurrentStationId(const CGraph<StationTrait<int>>& graph) const {
    (void)graph; // quitamos warning
    const auto& stIds = m_line->GetStations();
    // cambiamos el < a static_cast pa quitar el warning de signos
    if (stIds.empty() || static_cast<size_t>(m_currentStationIndex) >= stIds.size()) return -1;
    return stIds[m_currentStationIndex];
}