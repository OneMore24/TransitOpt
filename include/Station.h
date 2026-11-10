#ifndef __STATION_H__
#define __STATION_H__

#include <SFML/Graphics.hpp>
#include <string>

// traits pa la estasion
template <typename _T>
struct StationTrait {
    using IdType = _T;
};

template <typename Traits>
class CStation {
    using IdType = typename Traits::IdType;

private:
    IdType      m_id;
    size_t      m_gridX; // cordenada en la cuadricula
    size_t      m_gridY;
    sf::Vector2f m_pixelPos; // posicion en pixeles pa dibujar

public:
    CStation(IdType id, size_t x, size_t y, sf::Vector2f pixelPos)
        : m_id(id), m_gridX(x), m_gridY(y), m_pixelPos(pixelPos) {}

    IdType GetId() const { return m_id; }
    size_t GetGridX() const { return m_gridX; }
    size_t GetGridY() const { return m_gridY; }
    sf::Vector2f GetPixelPos() const { return m_pixelPos; }
    
    // aqui iran las conecciones despues
};

#endif // __STATION_H__