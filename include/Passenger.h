#ifndef __PASSENGER_H__
#define __PASSENGER_H__

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath> // pa usar sqrt
#include "Graph.h"

class CPassenger {
private:
    int m_id;
    int m_startId;
    int m_endId;
    std::vector<int> m_path; 
    size_t m_currentPathIndex; 
    sf::Vector2f m_pixelPos;
    sf::CircleShape m_shape;
    float m_speed;

public:
    CPassenger(int id, int start, int end, std::vector<int> path, sf::Vector2f startPos)
        : m_id(id), m_startId(start), m_endId(end), m_path(path), 
          m_currentPathIndex(0), m_pixelPos(startPos), m_speed(150.0f) { // vel a 150px/s
        
        m_shape.setRadius(5);
        m_shape.setOrigin(sf::Vector2f(5, 5));
        m_shape.setFillColor(sf::Color::Red);
        m_shape.setPosition(m_pixelPos);
    }

    // actualiza la posicion del pasajero frame a frame
    void Update(float deltaTime, const CGraph<StationTrait<int>>* graph) {
        if (HasArrived()) return;

        // obtenemos la estasion actual y la siguiente de su ruta
        int currentId = m_path[m_currentPathIndex];
        int nextId = m_path[m_currentPathIndex + 1];
        
        auto* stA = graph->GetStationById(currentId);
        auto* stB = graph->GetStationById(nextId);

        if (!stA || !stB) return; // por si algo falla

        sf::Vector2f targetPos = stB->GetPixelPos();
        sf::Vector2f dir = targetPos - m_pixelPos;
        
        // calculamos la distancia con pitagoras
        float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

        // si esta muy serca de la estasion, pasamos a la siguiente
        if (dist < 5.0f) {
            m_currentPathIndex++;
            m_pixelPos = targetPos;
        } else {
            // si no, avanzamos un poquito normalisando el vector
            dir /= dist; 
            m_pixelPos += dir * m_speed * deltaTime;
        }
        m_shape.setPosition(m_pixelPos);
    }
    
    void Draw(sf::RenderWindow& window) { window.draw(m_shape); }
    
    // si ya llego al final de su ruta
    bool HasArrived() const { return m_currentPathIndex >= m_path.size() - 1; }
};

#endif // __PASSENGER_H__