#ifndef __LINE_H__
#define __LINE_H__

#include <SFML/Graphics.hpp>
#include <vector>

class CLine {
private:
    int m_id;
    sf::Color m_color;
    std::vector<int> m_stationIds;

public:
    // Solo declaramos los metodos, sin llaves
    CLine(int id, sf::Color color);
    int GetId() const;
    sf::Color GetColor() const;
    const std::vector<int>& GetStations() const;
    void AddStation(int stationId);
    void ClearStations();
    bool HasStation(int stationId) const;
};

#endif