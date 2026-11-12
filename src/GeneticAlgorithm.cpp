#include "GeneticAlgorithm.h"
#include <algorithm>
#include <cmath>
#include <set> 
#include <map>
#include <thread>
#include <queue>
#include <numeric>
#include <iostream>

GeneticAlgorithm::GeneticAlgorithm(const std::vector<CStation<StationTrait<int>>*>& stations, 
                                   const CGrid<GridTrait<TerrainType>>& grid, 
                                   int maxBridges, int maxLines,
                                   int popSize, int gens)
    : m_stations(stations), m_grid(&grid), m_maxBridges(maxBridges), m_maxLines(maxLines),
      m_populationSize(popSize), m_generations(gens), m_currentGen(0), m_bestFitness(999999999.0f) {}

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
    
    int totalStations = m_stations.size();
    if (totalStations == 0) return 0.0f;

    std::vector<int> lineSizes;
    for (auto& line : c.lines) {
        lineSizes.push_back(line.GetStations().size());
        auto& stationsInLine = line.GetStations();
        for (size_t i = 0; i < stationsInLine.size() - 1; ++i) {
            int id1 = stationsInLine[i];
            int id2 = stationsInLine[i+1];
            CStation<StationTrait<int>>* stA = nullptr;
            CStation<StationTrait<int>>* stB = nullptr;
            for(auto* s : m_stations) {
                if(s->GetId() == id1) stA = s;
                if(s->GetId() == id2) stB = s;
            }
            if (!stA || !stB) continue;

            float dx = stA->GetPixelPos().x - stB->GetPixelPos().x;
            float dy = stA->GetPixelPos().y - stB->GetPixelPos().y;
            totalDistance += std::sqrt(dx*dx + dy*dy);

            bool cruzaRio = m_grid->IsRiverCrossing(stA->GetPixelPos().x, stA->GetPixelPos().y, stB->GetPixelPos().x, stB->GetPixelPos().y);
            if (cruzaRio) bridgesUsed++;

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

// Mutacion Adaptativa
void GeneticAlgorithm::Mutate(Chromosome& c) {
    if (c.lines.empty()) return;
    
    int tipoMutacion = rand() % 100;

    if (tipoMutacion < 40) {
        // 2-OPT (desenredar ruta)
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
        // Mover estacion entre lineas (Cross-Line)
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
        // Cambiar estacion por otra cercana
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

// --- NUEVO CRUCE POR CAMINOS (PATH CROSSOVER) ---
Chromosome GeneticAlgorithm::Crossover(const Chromosome& p1, const Chromosome& p2) {
    Chromosome child;
    if (p1.lines.empty()) return p2;
    if (p2.lines.empty()) return p1;

    for (int i = 0; i < m_maxLines; ++i) {
        // Elegimos al azar de que padre heredar esta linea
        const Chromosome& parent = (rand() % 2 == 0) ? p1 : p2;
        
        if (i >= static_cast<int>(parent.lines.size())) continue;
        
        CLine newLine(i + 1, sf::Color::Red);
        auto& stInLine = parent.lines[i].GetStations();
        
        if (stInLine.size() < 2) continue;

        // Tomamos la mitad de las estaciones del padre
        int cutPoint = stInLine.size() / 2;
        for (int j = 0; j < cutPoint; ++j) {
            newLine.AddStation(stInLine[j]);
        }
        
        // La otra mitad la tomamos del otro padre (si la tiene)
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
    
        // Aseguramos que el hijo tenga exactamente el numero de rutas pedido
    while (static_cast<int>(child.lines.size()) < m_maxLines && !p1.lines.empty()) {
        child.lines.push_back(p1.lines[rand() % p1.lines.size()]);
    }
    
    // CORRECCION: usamos pop_back() en lugar de resize() pa no crear CLine vacios
    while (static_cast<int>(child.lines.size()) > m_maxLines) {
        child.lines.pop_back();
    }

    return child;
}

std::vector<CLine> GeneticAlgorithm::Optimize() {
    std::cout << "iniciando IA con " << m_populationSize << " redes y " << m_generations << " generaciones...\n";
    std::vector<Chromosome> population(m_populationSize);
    for (int i = 0; i < m_populationSize; ++i) population[i] = CreateRandomChromosome();

    unsigned int numCores = std::thread::hardware_concurrency();
    if (numCores == 0) numCores = 4;

    float bestFitnessEver = 999999999.0f;
    int generationsWithoutImprovement = 0;

    for (int gen = 0; gen < m_generations; ++gen) {
        // Tasa de mutacion dinamica: Alta al principio, baja al final
        float mutationRate = 0.4f; // 40% inicial
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

        // actualisamos las variables atomicas pa la UI
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

        while (static_cast<int>(newPop.size()) < m_populationSize) {
            int p1 = rand() % 10;
            int p2 = rand() % 10;
            
            // Usamos el nuevo cruce inteligente
            Chromosome child = Crossover(population[p1], population[p2]);
            
            if ((rand() % 1000) / 1000.0f < mutationRate) Mutate(child);
            newPop.push_back(child);
        }
        population = newPop;
    }

    // --- REFINAMIENTO FINAL CON RECOCIDO SIMULADO ---
    std::cout << "aplicando refinamiento con recocido simulado...\n";
    Chromosome best = population[0];
    float bestFit = EvaluateChromosome(best);
    
    float temperature = 1000.0f; // Temperatura inicial alta
    float coolingRate = 0.99f;   // Factor de enfriamiento
    
    for (int iter = 0; iter < 1000; ++iter) {
        Chromosome test = best;
        Mutate(test);
        float fit = EvaluateChromosome(test);
        
        if (fit < bestFit) {
            // Mejor solucion: la aceptamos
            best = test;
            bestFit = fit;
        } else {
            // Peor solucion: la aceptamos con cierta probabilidad (depende de la temperatura)
            float delta = fit - bestFit;
            float acceptanceProb = std::exp(-delta / temperature);
            if ((rand() % 1000) / 1000.0f < acceptanceProb) {
                best = test;
                bestFit = fit;
            }
        }
        temperature *= coolingRate; // Enfriamiento
    }

    std::cout << "IA termino. Mejor fitness final: " << bestFit << "\n";
    return best.lines;
}