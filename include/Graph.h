#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "Station.h"
#include <iostream>

template <typename Traits>
class CGraph {
    using IdType = typename Traits::IdType;
    // tipo de estasion q usamos
    using StationType = CStation<StationTrait<IdType>>;

private:
    size_t m_capacity; // capasidad actual
    size_t m_numStations;
    StationType** m_pStations; // arreglo de punteros como en tus ramas

public:
    CGraph(size_t initialCapacity = 10);
    ~CGraph();

    void AddStation(IdType id, size_t x, size_t y, sf::Vector2f pixelPos);
    StationType* GetStation(size_t index) const;
    size_t GetNumStations() const { return m_numStations; }
    
    // busca si ya hay estasion en esa celda
    StationType* FindStationAt(size_t gridX, size_t gridY) const;
};

// --- IMPLEMENTACION ---

template <typename Traits>
CGraph<Traits>::CGraph(size_t initialCapacity) 
    : m_capacity(initialCapacity), m_numStations(0) {
    m_pStations = new StationType*[m_capacity];
    for (size_t i = 0; i < m_capacity; ++i) m_pStations[i] = nullptr;
}

template <typename Traits>
CGraph<Traits>::~CGraph() {
    for (size_t i = 0; i < m_numStations; ++i) {
        delete m_pStations[i]; // liberamos memoria
    }
    delete[] m_pStations; // liberamos el arreglo
}

template <typename Traits>
void CGraph<Traits>::AddStation(IdType id, size_t x, size_t y, sf::Vector2f pixelPos) {
    if (m_numStations >= m_capacity) {
        // redimencionamos manual xq es mas rapido q vectores
        size_t newCapacity = m_capacity * 2;
        StationType** newData = new StationType*[newCapacity];
        for (size_t i = 0; i < m_numStations; ++i) newData[i] = m_pStations[i];
        delete[] m_pStations;
        m_pStations = newData;
        m_capacity = newCapacity;
    }
    m_pStations[m_numStations++] = new StationType(id, x, y, pixelPos);
}

template <typename Traits>
typename CGraph<Traits>::StationType* CGraph<Traits>::GetStation(size_t index) const {
    if (index < m_numStations) return m_pStations[index];
    return nullptr;
}

template <typename Traits>
typename CGraph<Traits>::StationType* CGraph<Traits>::FindStationAt(size_t gridX, size_t gridY) const {
    for (size_t i = 0; i < m_numStations; ++i) {
        // comparamos las cordenadas pa ver si ya existe
        if (m_pStations[i]->GetGridX() == gridX && m_pStations[i]->GetGridY() == gridY) {
            return m_pStations[i];
        }
    }
    return nullptr;
}

#endif // __GRAPH_H__