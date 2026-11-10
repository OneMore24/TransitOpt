#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "Station.h"
#include <iostream>
#include <queue>
#include <vector>
#include <limits>
#include <algorithm>
#include <map>

// estructura pa las vias
struct CEdge {
    int m_idStationA;
    int m_idStationB;
    bool m_isBridge; // true si uso un puente
};

template <typename Traits>
class CGraph {
    using IdType = typename Traits::IdType;
    using StationType = CStation<StationTrait<IdType>>;

private:
    size_t m_capacity; 
    size_t m_numStations;
    StationType** m_pStations; 

    // arreglo de vias
    size_t m_edgeCapacity;
    size_t m_numEdges;
    CEdge* m_pEdges; // arreglo 1d de vias

public:
    CGraph(size_t initialCapacity = 10);
    ~CGraph();

    // algoritmo de dijkstra pa buscar la ruta mas corta
    std::vector<int> FindShortestPath(int startId, int endId) const;

    void AddStation(IdType id, size_t x, size_t y, sf::Vector2f pixelPos);
    StationType* GetStation(size_t index) const;
    StationType* FindStationAt(size_t gridX, size_t gridY) const;
    StationType* GetStationById(IdType id) const; // añadido pa buscar x id
    size_t GetNumStations() const { return m_numStations; }

    // metodos pa las vias
    void AddEdge(int idA, int idB, bool isBridge);
    size_t GetNumEdges() const { return m_numEdges; }
    CEdge GetEdge(size_t index) const { return m_pEdges[index]; }
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
    delete[] m_pEdges; // liberamos las vias
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

template <typename Traits>
typename CGraph<Traits>::StationType* CGraph<Traits>::GetStationById(IdType id) const {
    for (size_t i = 0; i < m_numStations; ++i) {
        if (m_pStations[i]->GetId() == id) {
            return m_pStations[i];
        }
    }
    return nullptr;
}

template <typename Traits>
void CGraph<Traits>::AddEdge(int idA, int idB, bool isBridge) {
    if (m_numEdges >= m_edgeCapacity) {
        // redimencionamos las vias
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
std::vector<int> CGraph<Traits>::FindShortestPath(int startId, int endId) const {
    std::map<int, float> dist;
    std::map<int, int> parent;
    
    // inicializamos las distancias al infinito
    for (size_t i = 0; i < m_numStations; ++i) {
        dist[m_pStations[i]->GetId()] = std::numeric_limits<float>::max();
    }
    dist[startId] = 0.0f;

    // cola de prioridad: guarda pares (distancia, id)
    // greater hace q sea un min-heap, sacando primero el mas sercano
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, std::greater<std::pair<float, int>>> pq;
    pq.push({0.0f, startId});

    while (!pq.empty()) {
        float d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (d > dist[u]) continue;
        if (u == endId) break; // ya llegamos

        // revisamos las vias conectadas a u
        for (size_t i = 0; i < m_numEdges; ++i) {
            CEdge edge = m_pEdges[i];
            int v = -1;
            if (edge.m_idStationA == u) v = edge.m_idStationB;
            else if (edge.m_idStationB == u) v = edge.m_idStationA;

            if (v != -1) {
                // el peso es la distancia en la grid (asumimos 1 x celda pa simplificar)
                // o podriamos usar la distancia euclidiana
                float weight = 1.0f; // peso base
                if (edge.m_isBridge) weight += 2.0f; // usar puente cuesta mas? (opcional)

                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
    }

    // reconstruimos el camino
    std::vector<int> path;
    if (dist[endId] == std::numeric_limits<float>::max()) return path; // no hay camino
    
    for (int at = endId; at != startId; at = parent[at]) {
        path.push_back(at);
    }
    path.push_back(startId);
    std::reverse(path.begin(), path.end());
    
    return path;
}

#endif // __GRAPH_H__