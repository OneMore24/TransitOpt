#include "GeneticAlgorithm.h"
#include <algorithm>
#include <cmath>
#include <set> 
#include <thread>

GeneticAlgorithm::GeneticAlgorithm(const std::vector<CStation<StationTrait<int>>*>& stations, 
                                   const CGrid<GridTrait<TerrainType>>& grid, 
                                   int maxBridges, int maxLines,
                                   int popSize, int gens)
    : m_stations(stations), m_grid(&grid), m_maxBridges(maxBridges), m_maxLines(maxLines),
      m_populationSize(popSize), m_generations(gens) {}

Chromosome GeneticAlgorithm::CreateRandomChromosome() {
    Chromosome c;
    if (m_stations.empty()) return c;

    int numLines = 1 + rand() % m_maxLines;
    
    std::vector<int> unvisitedStations;
    for (auto* st : m_stations) unvisitedStations.push_back(st->GetId());

    for (int i = 0; i < numLines; ++i) {
        CLine line(i + 1, sf::Color::Red);
        
        int currentId = -1;
        if (!unvisitedStations.empty()) {
            int idx = rand() % unvisitedStations.size();
            currentId = unvisitedStations[idx];
            unvisitedStations.erase(unvisitedStations.begin() + idx);
        } else {
            currentId = m_stations[rand() % m_stations.size()]->GetId();
        }

        line.AddStation(currentId);
        int numStations = 3 + rand() % 5; 

        for (int j = 0; j < numStations; ++j) {
            CStation<StationTrait<int>>* stActual = nullptr;
            for (auto* s : m_stations) if (s->GetId() == currentId) stActual = s;
            if (!stActual) break;

            std::vector<std::pair<float, int>> nearby;
            for (auto* st : m_stations) {
                if (!line.HasStation(st->GetId())) {
                    float dx = stActual->GetPixelPos().x - st->GetPixelPos().x;
                    float dy = stActual->GetPixelPos().y - st->GetPixelPos().y;
                    float dist = std::sqrt(dx*dx + dy*dy);
                    nearby.push_back({dist, st->GetId()});
                }
            }

            if (nearby.empty()) break;
            std::sort(nearby.begin(), nearby.end());

            int pickIdx = rand() % std::min(static_cast<int>(nearby.size()), 3);
            int nextId = nearby[pickIdx].second;
            
            line.AddStation(nextId);
            currentId = nextId;
            
            auto it = std::find(unvisitedStations.begin(), unvisitedStations.end(), nextId);
            if (it != unvisitedStations.end()) unvisitedStations.erase(it);
        }
        c.lines.push_back(line);
    }
    return c;
}

float GeneticAlgorithm::EvaluateChromosome(Chromosome& c) {
    int bridgesUsed = 0;
    std::set<int> uniqueConnectedStations; 
    float totalDistance = 0.0f;

    for (auto& line : c.lines) {
        auto& stationsInLine = line.GetStations();
        for (int stId : stationsInLine) uniqueConnectedStations.insert(stId);
        
        for (size_t i = 0; i < stationsInLine.size() - 1; ++i) {
            // CORRECCION AQUI: le decimos explicitamente el tipo de puntero
            CStation<StationTrait<int>>* stA = nullptr;
            CStation<StationTrait<int>>* stB = nullptr;

            for(auto* s : m_stations) {
                if(s->GetId() == stationsInLine[i]) stA = s;
                if(s->GetId() == stationsInLine[i+1]) stB = s;
            }
            if (!stA || !stB) continue;

            float dx = stA->GetPixelPos().x - stB->GetPixelPos().x;
            float dy = stA->GetPixelPos().y - stB->GetPixelPos().y;
            totalDistance += std::sqrt(dx*dx + dy*dy);

            bool cruzaRio = m_grid->IsRiverCrossing(stA->GetPixelPos().x, stA->GetPixelPos().y, stB->GetPixelPos().x, stB->GetPixelPos().y);
            if (cruzaRio) {
                bridgesUsed++;
                if (bridgesUsed > m_maxBridges) return 99999999.0f; 
            }
        }
    }

    int totalStations = m_stations.size();
    int disconnected = totalStations - uniqueConnectedStations.size();
    float penaltyDisconnected = disconnected * 50000.0f; 

    float penaltyLines = c.lines.size() * 2000.0f;

    return totalDistance + penaltyDisconnected + penaltyLines;
}

void GeneticAlgorithm::Mutate(Chromosome& c) {
    if (c.lines.empty()) return;
    int lineIdx = rand() % c.lines.size();
    auto& stations = const_cast<std::vector<int>&>(c.lines[lineIdx].GetStations());
    
    if (stations.size() > 2 && rand() % 100 < 30) {
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
                int pickIdx = rand() % std::min(static_cast<int>(nearby.size()), 3);
                stations[idxToChange] = nearby[pickIdx].second;
            }
        }
    }
}

std::vector<CLine> GeneticAlgorithm::Optimize() {
    std::cout << "iniciando IA con " << m_populationSize << " redes y " << m_generations << " generaciones...\n";
    std::vector<Chromosome> population(m_populationSize);
    for (int i = 0; i < m_populationSize; ++i) population[i] = CreateRandomChromosome();

    unsigned int numCores = std::thread::hardware_concurrency();
    if (numCores == 0) numCores = 4;

    for (int gen = 0; gen < m_generations; ++gen) {
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

        std::vector<Chromosome> newPop;
        for (int i = 0; i < 10 && i < m_populationSize; ++i) newPop.push_back(population[i]);

        while (static_cast<int>(newPop.size()) < m_populationSize) {
            int p1 = rand() % 10;
            int p2 = rand() % 10;
            
            Chromosome child;
            if (!population[p1].lines.empty()) child.lines.push_back(population[p1].lines[0]);
            if (!population[p2].lines.empty()) child.lines.push_back(population[p2].lines[0]);
            
            for (auto& line : population[p1].lines) {
                if (rand() % 2 == 0 && static_cast<int>(child.lines.size()) < m_maxLines) {
                    child.lines.push_back(line);
                }
            }
            
            if (rand() % 100 < 20) Mutate(child);
            newPop.push_back(child);
        }
        population = newPop;
    }

    for (int i = 0; i < m_populationSize; ++i) population[i].fitness = EvaluateChromosome(population[i]);
    std::sort(population.begin(), population.end(), [](const Chromosome& a, const Chromosome& b) { return a.fitness < b.fitness; });

    std::cout << "IA termino. Mejor fitness: " << population[0].fitness << "\n";
    return population[0].lines;
}