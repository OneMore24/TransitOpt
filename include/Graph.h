#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "Station.h"
#include <iostream>
#include <queue>
#include <vector>
#include <limits>
#include <algorithm>
#include <map>
#include <cmath>

// estructura pa las vias
struct CEdge {
    int m_idStationA;
    int m_idStationB;
    bool m_isBridge;
};

template <typename Traits>
class CGraph {
    using IdType = typename Traits::IdType;
    using StationType = CStation<StationTrait<IdType>>;

private:
    size_t m_capacity; 
    size_t m_numStations;
    StationType** m_pStations; 

    size_t m_edgeCapacity;
    size_t m_numEdges;
    CEdge* m_pEdges; 

public:
    CGraph(size_t initialCapacity = 10);
    ~CGraph();

    void AddStation(IdType id, size_t x, size_t y, sf::Vector2f pixelPos, ::StationType type);
    StationType* GetStation(size_t index) const;
    StationType* FindStationAt(size_t gridX, size_t gridY) const;
    StationType* GetStationById(IdType id) const; 
    size_t GetNumStations() const { return m_numStations; }

    void AddEdge(int idA, int idB, bool isBridge);
    void ClearEdges() { m_numEdges = 0; }
    size_t GetNumEdges() const { return m_numEdges; }
    CEdge GetEdge(size_t index) const { return m_pEdges[index]; }

    // metodo seguro pa limpiar el mapa sin dejar puntetos colgados
    void Clear() {
        for (size_t i = 0; i < m_numStations; ++i) {
            delete m_pStations[i];
            m_pStations[i] = nullptr;
        }
        m_numStations = 0;
        m_numEdges = 0;
    }

    std::vector<int> FindShortestPathToType(int startId, ::StationType targetType) const;
};

// --- IMPLEMENTACION ---

template <typename Traits>
CGraph<Traits>::CGraph(size_t initialCapacity) 
    : m_capacity(initialCapacity), m_numStations(0),
      m_edgeCapacity(10), m_numEdges(0) {
    m_pStations = new StationType*[m_capacity];
    for (size_t i = 0; i < m_capacity; ++i) m_pStations[i] = nullptr;
    m_pEdges = new CEdge[m_edgeCapacity];
}

template <typename Traits>
CGraph<Traits>::~CGraph() {
    for (size_t i = 0; i < m_numStations; ++i) delete m_pStations[i];
    delete[] m_pStations; 
    delete[] m_pEdges; 
}

template <typename Traits>
void CGraph<Traits>::AddStation(IdType id, size_t x, size_t y, sf::Vector2f pixelPos, ::StationType type) {
    if (m_numStations >= m_capacity) {
        size_t newCapacity = m_capacity * 2;
        StationType** newData = new StationType*[newCapacity];
        for (size_t i = 0; i < m_numStations; ++i) newData[i] = m_pStations[i];
        delete[] m_pStations;
        m_pStations = newData;
        m_capacity = newCapacity;
    }
    m_pStations[m_numStations++] = new StationType(id, x, y, pixelPos, type);
}

template <typename Traits>
typename CGraph<Traits>::StationType* CGraph<Traits>::GetStation(size_t index) const {
    if (index < m_numStations) return m_pStations[index];
    return nullptr;
}

template <typename Traits>
typename CGraph<Traits>::StationType* CGraph<Traits>::FindStationAt(size_t gridX, size_t gridY) const {
    for (size_t i = 0; i < m_numStations; ++i) {
        if (m_pStations[i]->GetGridX() == gridX && m_pStations[i]->GetGridY() == gridY) {
            return m_pStations[i];
        }
    }
    return nullptr;
}

template <typename Traits>
typename CGraph<Traits>::StationType* CGraph<Traits>::GetStationById(IdType id) const {
    for (size_t i = 0; i < m_numStations; ++i) {
        if (m_pStations[i]->GetId() == id) return m_pStations[i];
    }
    return nullptr;
}

template <typename Traits>
void CGraph<Traits>::AddEdge(int idA, int idB, bool isBridge) {
    if (m_numEdges >= m_edgeCapacity) {
        size_t newCapacity = m_edgeCapacity * 2;
        CEdge* newData = new CEdge[newCapacity];
        for (size_t i = 0; i < m_numEdges; ++i) newData[i] = m_pEdges[i];
        delete[] m_pEdges;
        m_pEdges = newData;
        m_edgeCapacity = newCapacity;
    }
    m_pEdges[m_numEdges++] = {idA, idB, isBridge};
}

template <typename Traits>
std::vector<int> CGraph<Traits>::FindShortestPathToType(int startId, ::StationType targetType) const {
    std::map<int, float> dist;
    std::map<int, int> parent;
    
    for (size_t i = 0; i < m_numStations; ++i) {
        dist[m_pStations[i]->GetId()] = std::numeric_limits<float>::max();
    }
    dist[startId] = 0.0f;

    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::greater<std::pair<float, int>>> pq;
    pq.push({0.0f, startId});

    int targetStationId = -1;

    while (!pq.empty()) {
        float d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (d > dist[u]) continue;

        auto* stU = GetStationById(u);
        if (stU->GetType() == targetType && u != startId) {
            targetStationId = u;
            break;
        }

        for (size_t i = 0; i < m_numEdges; ++i) {
            CEdge edge = m_pEdges[i];
            int v = -1;
            if (edge.m_idStationA == u) v = edge.m_idStationB;
            else if (edge.m_idStationB == u) v = edge.m_idStationA;

            if (v != -1) {
                auto* stV = GetStationById(v);
                float dx = stU->GetPixelPos().x - stV->GetPixelPos().x;
                float dy = stU->GetPixelPos().y - stV->GetPixelPos().y;
                float weight = std::sqrt(dx*dx + dy*dy) / 30.0f;

                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
    }

    std::vector<int> path;
    if (targetStationId == -1) return path; 
    
    for (int at = targetStationId; at != startId; at = parent[at]) {
        path.push_back(at);
    }
    path.push_back(startId);
    std::reverse(path.begin(), path.end());
    return path;
}

#endif