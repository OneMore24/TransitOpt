#ifndef __PASSENGER_H__
#define __PASSENGER_H__

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include "Graph.h"

class CPassenger {
private:
    int m_id;
    int m_startId;
    StationType m_targetType;
    std::vector<int> m_path; 
    size_t m_currentPathIndex; 
    sf::Vector2f m_pixelPos;
    sf::CircleShape m_shape;
    float m_speed;

public:
    CPassenger(int id, int start, StationType target, std::vector<int> path, sf::Vector2f startPos)
        : m_id(id), m_startId(start), m_targetType(target), m_path(path), 
          m_currentPathIndex(0), m_pixelPos(startPos), m_speed(150.0f) {
        
        m_shape.setRadius(4);
        m_shape.setOrigin(sf::Vector2f(4, 4));
        m_shape.setFillColor(sf::Color::Red);
        m_shape.setPosition(m_pixelPos);
    }

    void Update(float deltaTime, CGraph<StationTrait<int>>* graph) {
        if (HasArrived()) return;

        int currentId = m_path[m_currentPathIndex];
        int nextId = m_path[m_currentPathIndex + 1];
        
        auto* stA = graph->GetStationById(currentId);
        auto* stB = graph->GetStationById(nextId);

        if (!stA || !stB) return; 

        sf::Vector2f targetPos = stB->GetPixelPos();
        sf::Vector2f dir = targetPos - m_pixelPos;
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

        if (dist < 5.0f) {
            m_currentPathIndex++;
            m_pixelPos = targetPos;
        } else {
            dir /= dist; 
            m_pixelPos += dir * m_speed * deltaTime;
        }
        m_shape.setPosition(m_pixelPos);
    }
    
    void Draw(sf::RenderWindow& window) { window.draw(m_shape); }
    
    bool HasArrived() const { return m_currentPathIndex >= m_path.size() - 1; }

    // ESTE ERA EL METODO QUE FALTABA
    sf::Vector2f GetPixelPos() const { return m_pixelPos; }
};

#endif // __PASSENGER_H__