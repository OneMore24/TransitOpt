#ifndef __PASSENGER_H__
#define __PASSENGER_H__

#include <SFML/Graphics.hpp>
#include <vector>
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
    CPassenger(int id, int start, StationType target, std::vector<int> path, sf::Vector2f startPos);
    void Update(float deltaTime, CGraph<StationTrait<int>>* graph);
    void Draw(sf::RenderWindow& window);
    bool HasArrived() const;
    sf::Vector2f GetPixelPos() const;
};

#endif