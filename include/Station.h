#ifndef __STATION_H__
#define __STATION_H__

#include <SFML/Graphics.hpp>
#include <string>

// los 10 tipos de estasion
enum class StationType {
    CIRCULO, CUADRADO, TRIANGULO, OVALO, PENTAGONO, 
    CONO, DIAMANTE, ESTRELLA, RECTANGULO, CRUZ
};

template <typename _T>
struct StationTrait {
    using IdType = _T;
};

template <typename Traits>
class CStation {
    using IdType = typename Traits::IdType;

private:
    IdType      m_id;
    size_t      m_gridX;
    size_t      m_gridY;
    sf::Vector2f m_pixelPos;
    StationType m_type; // el tipo de estasion (figura)

public:
    CStation(IdType id, size_t x, size_t y, sf::Vector2f pixelPos, StationType type)
        : m_id(id), m_gridX(x), m_gridY(y), m_pixelPos(pixelPos), m_type(type) {}

    IdType GetId() const { return m_id; }
    size_t GetGridX() const { return m_gridX; }
    size_t GetGridY() const { return m_gridY; }
    sf::Vector2f GetPixelPos() const { return m_pixelPos; }
    StationType GetType() const { return m_type; }
};

#endif // __STATION_H__