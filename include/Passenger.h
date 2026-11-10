#ifndef __PASSENGER_H__
#define __PASSENGER_H__

#include <SFML/Graphics.hpp>
#include <vector>

class CPassenger {
private:
    int m_id;
    int m_startId;
    int m_endId;
    std::vector<int> m_path; // la ruta de ids q le dio dijkstra
    size_t m_currentPathIndex; // en q estasion de la ruta va
    sf::Vector2f m_pixelPos;
    sf::CircleShape m_shape;
    float m_speed;

public:
    CPassenger(int id, int start, int end, std::vector<int> path, sf::Vector2f startPos)
        : m_id(id), m_startId(start), m_endId(end), m_path(path), 
          m_currentPathIndex(0), m_pixelPos(startPos), m_speed(100.0f) {
        
        m_shape.setRadius(5);
        m_shape.setOrigin(sf::Vector2f(5, 5));
        m_shape.setFillColor(sf::Color::Red); // el pasajero es un punto rojo
        m_shape.setPosition(m_pixelPos);
    }

    // actualiza la posicion del pasajero
    void Update(float deltaTime, const class CGraph<struct StationTrait<int>>* graph);
    
    void Draw(sf::RenderWindow& window) { window.draw(m_shape); }
    
    bool HasArrived() const { return m_currentPathIndex >= m_path.size() - 1; }
};

#endif // __PASSENGER_H__