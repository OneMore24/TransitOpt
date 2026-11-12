#include "GeneticAlgorithm.h"
#include <algorithm>
#include <cmath>
#include <set> 
#include <map>
#include <thread>
#include <queue>
#include <numeric>
#include <iostream>
#include <functional>
#include <unordered_set>

// Estructura Union-Find pal algoritmo de Kruskal
struct DSU {
    std::unordered_map<int, int> parent;
    int find(int x) {
        if (parent.find(x) == parent.end()) parent[x] = x;
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }
    void unite(int x, int y) {
        int px = find(x), py = find(y);
        if (px != py) parent[px] = py;
    }
    bool connected(int x, int y) { return find(x) == find(y); }
};

GeneticAlgorithm::GeneticAlgorithm(const std::vector<CStation<StationTrait<int>>*>& stations, 
                                   const CGrid<GridTrait<TerrainType>>& grid, 
                                   int maxBridges, int maxLines,
                                   int popSize, int gens)
    : m_stations(stations), m_grid(&grid), m_maxBridges(maxBridges), m_maxLines(maxLines),
      m_populationSize(popSize), m_generations(gens),
      m_currentGen(0), m_bestFitness(999999999.0f) {}

// Genera una semilla usando Kruskal (Arbol de Expansion Minima)
void GeneticAlgorithm::GenerateMSTSeed() {
    m_seedLines.clear();
    m_mstCost = 0.0f;
    if (m_stations.size() < 2) return;

    // 1. Creamos todas las aristas posibles con su distancia
    struct Edge { float weight; float distanceOnly; int u, v; };
    std::vector<Edge> allEdges;

    for (size_t i = 0; i < m_stations.size(); ++i) {
        for (size_t j = i + 1; j < m_stations.size(); ++j) {
            int id1 = m_stations[i]->GetId();
            int id2 = m_stations[j]->GetId();
            float dx = m_stations[i]->GetPixelPos().x - m_stations[j]->GetPixelPos().x;
            float dy = m_stations[i]->GetPixelPos().y - m_stations[j]->GetPixelPos().y;
            float dist = std::sqrt(dx*dx + dy*dy);
            bool cruza = m_grid->IsRiverCrossing(
                m_stations[i]->GetPixelPos().x, m_stations[i]->GetPixelPos().y,
                m_stations[j]->GetPixelPos().x, m_stations[j]->GetPixelPos().y
            );
            // El MST prefiere no cruzar el rio (con un pequeño castigo)
            float weight = dist + (cruza ? 50.0f : 0.0f);
            allEdges.push_back({weight, dist, id1, id2});
        }
    }

    // 2. Ordenamos por peso y aplicamos Kruskal
    std::sort(allEdges.begin(), allEdges.end(), [](const Edge& a, const Edge& b) { return a.weight < b.weight; });

    DSU dsu;
    std::vector<Edge> mstEdges;
    for (auto& e : allEdges) {
        if (!dsu.connected(e.u, e.v)) {
            dsu.unite(e.u, e.v);
            mstEdges.push_back(e);
            m_mstCost += e.distanceOnly; // sumamos solo la distancia (sin castigo)
        }
    }

    // 3. Construimos la lista de adyacencia del MST
    std::map<int, std::vector<int>> adj;
    for (auto& e : mstEdges) {
        adj[e.u].push_back(e.v);
        adj[e.v].push_back(e.u);
    }

    // 4. Recorrido DFS pa generar un "tour" que visite todas las estaciones
    std::vector<int> tour;
    std::set<int> visited;
    std::function<void(int)> dfs = [&](int u) {
        visited.insert(u);
        tour.push_back(u);
        for (int v : adj[u]) {
            if (!visited.count(v)) dfs(v);
        }
    };
    dfs(m_stations[0]->GetId());

    // 5. Partimos el tour en m_maxLines lineas
    if (tour.size() < 2) return;
    int chunkSize = std::max(2, static_cast<int>(std::ceil(static_cast<float>(tour.size()) / m_maxLines)));
    
    int lineId = 1;
    for (size_t i = 0; i < tour.size() && lineId <= m_maxLines; i += chunkSize) {
        CLine line(lineId++, sf::Color::Red);
        for (size_t j = i; j < std::min(i + chunkSize + 1, tour.size()); ++j) {
            if (!line.HasStation(tour[j])) line.AddStation(tour[j]);
        }
        if (line.GetStations().size() >= 2) m_seedLines.push_back(line);
    }
}

Chromosome GeneticAlgorithm::CreateRandomChromosome() {
    Chromosome c;
    if (m_stations.empty()) return c;

    int numLines = m_maxLines;
    if (m_stations.size() < static_cast<size_t>(numLines * 2)) {
        numLines = std::max(1, static_cast<int>(m_stations.size() / 2));
    }

    for (int i = 0; i < numLines; ++i) {
        CLine line(i + 1, sf::Color::Red);
        int currentIdx = rand() % m_stations.size();
        CStation<StationTrait<int>>* currentSt = m_stations[currentIdx];
        line.AddStation(currentSt->GetId());

        int stationsPerLine = std::max(2, static_cast<int>(m_stations.size() / (numLines * 0.8f)));
        if (stationsPerLine > 25) stationsPerLine = 25; 

        for (int j = 0; j < stationsPerLine; ++j) {
            std::vector<std::pair<float, CStation<StationTrait<int>>*>> nearby;
            for (size_t k = 0; k < m_stations.size(); ++k) {
                if (!line.HasStation(m_stations[k]->GetId())) {
                    float dx = currentSt->GetPixelPos().x - m_stations[k]->GetPixelPos().x;
                    float dy = currentSt->GetPixelPos().y - m_stations[k]->GetPixelPos().y;
                    nearby.push_back({std::sqrt(dx*dx + dy*dy), m_stations[k]});
                }
            }
            if (nearby.empty()) break;
            std::sort(nearby.begin(), nearby.end(), [](const auto& a, const auto& b){ return a.first < b.first; });
            int pick = rand() % std::min(static_cast<int>(nearby.size()), 3);
            CStation<StationTrait<int>>* chosenSt = nearby[pick].second;
            line.AddStation(chosenSt->GetId());
            currentSt = chosenSt;
        }
        if (line.GetStations().size() >= 2) c.lines.push_back(line);
    }
    return c;
}

float GeneticAlgorithm::EvaluateChromosome(Chromosome& c) {
    float totalDistance = 0.0f;
    int bridgesUsed = 0;
    std::set<std::pair<int, int>> uniqueEdges;
    std::map<int, std::vector<int>> adj;
    
    size_t totalStations = m_stations.size();
    if (totalStations == 0) return 0.0f;

    std::vector<int> lineSizes;
    for (auto& line : c.lines) {
        lineSizes.push_back(line.GetStations().size());
        auto& stationsInLine = line.GetStations();
        if (stationsInLine.size() < 2) continue;

        for (size_t i = 0; i < stationsInLine.size() - 1; ++i) {
            int id1 = stationsInLine[i];
            int id2 = stationsInLine[i+1];
            
            auto itA = m_idToIndex.find(id1);
            auto itB = m_idToIndex.find(id2);
            if (itA == m_idToIndex.end() || itB == m_idToIndex.end()) continue;

            size_t idxA = itA->second;
            size_t idxB = itB->second;

            totalDistance += m_distMatrix[idxA][idxB];
            if (m_riverMatrix[idxA][idxB]) bridgesUsed++;

            adj[id1].push_back(id2);
            adj[id2].push_back(id1);
            
            auto edge = std::make_pair(std::min(id1, id2), std::max(id1, id2));
            if (uniqueEdges.count(edge)) totalDistance += 10000.0f; 
            uniqueEdges.insert(edge);
        }
    }

    float penaltyBridges = 0.0f;
    int diffPuentes = bridgesUsed - m_maxBridges;
    if (diffPuentes != 0) penaltyBridges = std::abs(diffPuentes) * 200000.0f;

    float penaltyImbalance = 0.0f;
    if (!lineSizes.empty()) {
        int maxSize = *std::max_element(lineSizes.begin(), lineSizes.end());
        int minSize = *std::min_element(lineSizes.begin(), lineSizes.end());
        if (maxSize - minSize > 10) penaltyImbalance = (maxSize - minSize - 10) * 5000.0f;
    }

    int isolatedStations = 0;
    if (!m_stations.empty()) {
        std::set<int> visited;
        std::queue<int> q;
        q.push(m_stations[0]->GetId());
        visited.insert(m_stations[0]->GetId());
        while(!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : adj[u]) {
                if (!visited.count(v)) { visited.insert(v); q.push(v); }
            }
        }
        isolatedStations = totalStations - visited.size();
    }
    float penaltyDisconnected = isolatedStations * 500000.0f;

    return totalDistance + penaltyBridges + penaltyImbalance + penaltyDisconnected;
}

void GeneticAlgorithm::Mutate(Chromosome& c) {
    if (c.lines.empty()) return;
    
    int tipoMutacion = rand() % 100;

    if (tipoMutacion < 40) {
        int lineIdx = rand() % c.lines.size();
        auto& stations = const_cast<std::vector<int>&>(c.lines[lineIdx].GetStations());
        if (stations.size() > 3) {
            int i = rand() % stations.size();
            int j = rand() % stations.size();
            if (i > j) std::swap(i, j);
            if (i != j && (j - i) > 1) std::reverse(stations.begin() + i, stations.begin() + j);
        }
    }
    else if (tipoMutacion < 70 && c.lines.size() > 1) {
        int lineA = rand() % c.lines.size();
        int lineB = rand() % c.lines.size();
        if (lineA != lineB) {
            auto& stationsA = const_cast<std::vector<int>&>(c.lines[lineA].GetStations());
            auto& stationsB = const_cast<std::vector<int>&>(c.lines[lineB].GetStations());
            
            if (stationsA.size() > 2) {
                int idxToMove = rand() % stationsA.size();
                int stationToMove = stationsA[idxToMove];
                stationsA.erase(stationsA.begin() + idxToMove);
                int insertPos = rand() % (stationsB.size() + 1);
                stationsB.insert(stationsB.begin() + insertPos, stationToMove);
            }
        }
    }
    else {
        int lineIdx = rand() % c.lines.size();
        auto& stations = const_cast<std::vector<int>&>(c.lines[lineIdx].GetStations());
        if (stations.size() > 2) {
            int idxToChange = rand() % stations.size();
            int neighborId = (idxToChange > 0) ? stations[idxToChange - 1] : stations[1];
            
            CStation<StationTrait<int>>* stNeighbor = nullptr;
            for (auto* s : m_stations) if (s->GetId() == neighborId) stNeighbor = s;
            
            if (stNeighbor) {
                std::vector<std::pair<float, int>> nearby;
                for (auto* st : m_stations) {
                    if (!c.lines[lineIdx].HasStation(st->GetId())) {
                        float dx = stNeighbor->GetPixelPos().x - st->GetPixelPos().x;
                        float dy = stNeighbor->GetPixelPos().y - st->GetPixelPos().y;
                        nearby.push_back({std::sqrt(dx*dx + dy*dy), st->GetId()});
                    }
                }
                std::sort(nearby.begin(), nearby.end());
                if (!nearby.empty()) {
                    int pickIdx = rand() % std::min(static_cast<int>(nearby.size()), 5);
                    stations[idxToChange] = nearby[pickIdx].second;
                }
            }
        }
    }
}

Chromosome GeneticAlgorithm::Crossover(const Chromosome& p1, const Chromosome& p2) {
    Chromosome child;
    if (p1.lines.empty()) return p2;
    if (p2.lines.empty()) return p1;

    for (int i = 0; i < m_maxLines; ++i) {
        const Chromosome& parent = (rand() % 2 == 0) ? p1 : p2;
        
        if (i >= static_cast<int>(parent.lines.size())) continue;
        
        CLine newLine(i + 1, sf::Color::Red);
        auto& stInLine = parent.lines[i].GetStations();
        
        if (stInLine.size() < 2) continue;

        int cutPoint = stInLine.size() / 2;
        for (int j = 0; j < cutPoint; ++j) {
            newLine.AddStation(stInLine[j]);
        }
        
        const Chromosome& otherParent = (rand() % 2 == 0) ? p1 : p2;
        if (i < static_cast<int>(otherParent.lines.size())) {
            auto& otherSt = otherParent.lines[i].GetStations();
            for (size_t j = cutPoint; j < otherSt.size(); ++j) {
                if (!newLine.HasStation(otherSt[j])) {
                    newLine.AddStation(otherSt[j]);
                }
            }
        }
        if (newLine.GetStations().size() >= 2) child.lines.push_back(newLine);
    }
    
    while (static_cast<int>(child.lines.size()) < m_maxLines && !p1.lines.empty()) {
        child.lines.push_back(p1.lines[rand() % p1.lines.size()]);
    }
    
    while (static_cast<int>(child.lines.size()) > m_maxLines) {
        child.lines.pop_back();
    }

    return child;
}

std::vector<CLine> GeneticAlgorithm::Optimize() {
    std::cout << "iniciando IA con " << m_populationSize << " redes y " << m_generations << " generaciones...\n";
    
    // --- PRECALCULAMOS LAS MATRICES ---
    size_t n = m_stations.size();
    m_idToIndex.clear();
    for (size_t i = 0; i < n; ++i) {
        m_idToIndex[m_stations[i]->GetId()] = i;
    }

    m_distMatrix.assign(n, std::vector<float>(n, 0.0f));
    m_riverMatrix.assign(n, std::vector<bool>(n, false));
    
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            float dx = m_stations[i]->GetPixelPos().x - m_stations[j]->GetPixelPos().x;
            float dy = m_stations[i]->GetPixelPos().y - m_stations[j]->GetPixelPos().y;
            m_distMatrix[i][j] = std::sqrt(dx*dx + dy*dy);
            m_riverMatrix[i][j] = m_grid->IsRiverCrossing(
                m_stations[i]->GetPixelPos().x, m_stations[i]->GetPixelPos().y,
                m_stations[j]->GetPixelPos().x, m_stations[j]->GetPixelPos().y
            );
        }
    }

    // --- GENERAMOS EL MST COMO BENCHMARK Y SEMILLA ---
    GenerateMSTSeed();
    std::cout << "MST calculado. Costo teorico minimo (sin restricciones): " << m_mstCost << "\n";
    std::cout << "Semilla MST generada con " << m_seedLines.size() << " lineas\n";

    std::vector<Chromosome> population(m_populationSize);
    for (int i = 0; i < m_populationSize; ++i) {
        // El 20% inicial sera el MST mutado (buen punto de partida)
        if (i < m_populationSize * 0.2f && !m_seedLines.empty()) {
            population[i].lines = m_seedLines;
            for (int k = 0; k < 3; ++k) Mutate(population[i]);
        } else {
            population[i] = CreateRandomChromosome();
        }
    }

    unsigned int numCores = std::thread::hardware_concurrency();
    if (numCores == 0) numCores = 4;

    float bestFitnessEver = 999999999.0f;
    int generationsWithoutImprovement = 0;

    for (int gen = 0; gen < m_generations; ++gen) {
        float mutationRate = 0.4f;
        if (gen > m_generations / 2) mutationRate = 0.15f;
        if (gen > m_generations * 0.8f) mutationRate = 0.05f;

        for (int i = 0; i < m_populationSize; i += numCores) {
            std::vector<std::future<float>> futures;
            int batchEnd = std::min(i + static_cast<int>(numCores), m_populationSize);
            for (int j = i; j < batchEnd; ++j) {
                futures.push_back(std::async(std::launch::async, &GeneticAlgorithm::EvaluateChromosome, this, std::ref(population[j])));
            }
            for (size_t j = 0; j < futures.size(); ++j) {
                population[i + j].fitness = futures[j].get();
            }
        }

        std::sort(population.begin(), population.end(), [](const Chromosome& a, const Chromosome& b) { return a.fitness < b.fitness; });

        m_currentGen = gen;
        m_bestFitness = population[0].fitness;

        if (population[0].fitness < bestFitnessEver - 0.01f) {
            bestFitnessEver = population[0].fitness;
            generationsWithoutImprovement = 0;
        } else {
            generationsWithoutImprovement++;
        }

        if (generationsWithoutImprovement >= 40) {
            std::cout << "estancado en gen " << gen << ". inyectando inmigrantes...\n";
            int numImmigrants = m_populationSize * 0.2f;
            for (int i = 0; i < numImmigrants; ++i) {
                population[m_populationSize - 1 - i] = CreateRandomChromosome();
            }
            generationsWithoutImprovement = 0;
        }

        std::vector<Chromosome> newPop;
        for (int i = 0; i < 10 && i < m_populationSize; ++i) newPop.push_back(population[i]);

        auto tournamentSelect = [&]() {
            int bestIdx = rand() % m_populationSize;
            for (int k = 1; k < 3; ++k) {
                int idx = rand() % m_populationSize;
                if (population[idx].fitness < population[bestIdx].fitness) bestIdx = idx;
            }
            return bestIdx;
        };

        while (static_cast<int>(newPop.size()) < m_populationSize) {
            int p1 = tournamentSelect();
            int p2 = tournamentSelect();
            
            Chromosome child = Crossover(population[p1], population[p2]);
            
            if ((rand() % 1000) / 1000.0f < mutationRate) Mutate(child);
            newPop.push_back(child);
        }
        population = newPop;
    }

    // --- REFINAMIENTO CON RECOCIDO SIMULADO ---
    std::cout << "aplicando refinamiento con recocido simulado...\n";
    
    Chromosome current = population[0];
    float currentFit = EvaluateChromosome(current);
    Chromosome best = current;
    float bestFit = currentFit;
    
    float temperature = 1000.0f;
    float coolingRate = 0.99f;
    
    for (int iter = 0; iter < 1000; ++iter) {
        Chromosome test = current;
        Mutate(test);
        float fit = EvaluateChromosome(test);
        
        if (fit < currentFit) {
            current = test;
            currentFit = fit;
        } else {
            float delta = fit - currentFit;
            float acceptanceProb = std::exp(-delta / temperature);
            if ((rand() % 1000) / 1000.0f < acceptanceProb) {
                current = test;
                currentFit = fit;
            }
        }
        
        if (currentFit < bestFit) {
            best = current;
            bestFit = currentFit;
        }
        temperature *= coolingRate;
    }

    // --- BENCHMARK ---
    std::cout << "\n=========================================\n";
    std::cout << "BENCHMARK FINAL:\n";
    std::cout << "  Costo MST (cota inferior teorica): " << m_mstCost << "\n";
    std::cout << "  Fitness de la IA:                  " << bestFit << "\n";
    if (m_mstCost > 0.0f) {
        float gap = bestFit / m_mstCost;
        std::cout << "  Brecha (IA / MST):                 " << gap << "x\n";
        if (gap < 1.5f) {
            std::cout << "  VEREDICTO: EXCELENTE (dentro del 50% del optimo)\n";
        } else if (gap < 2.5f) {
            std::cout << "  VEREDICTO: BUENO (dentro del 150% del optimo)\n";
        } else {
            std::cout << "  VEREDICTO: MEJORABLE (lejos del optimo, revisar algoritmo)\n";
        }
    }
    std::cout << "=========================================\n\n";

    return best.lines;
}