#include "GeneticAlgorithm.h"
#include <algorithm>
#include <cmath>

GeneticAlgorithm::GeneticAlgorithm(const std::vector<CStation<StationTrait<int>>*>& stations, 
                                   const CGrid<GridTrait<TerrainType>>& grid, 
                                   int maxBridges, int maxLines,
                                   const std::vector<std::pair<int, StationType>>& demands,
                                   int popSize, int gens)
    : m_stations(stations), m_grid(&grid), m_maxBridges(maxBridges), m_maxLines(maxLines),
      m_demands(demands), m_populationSize(popSize), m_generations(gens) {}

Chromosome GeneticAlgorithm::CreateRandomChromosome() {
    Chromosome c;
    if (m_stations.empty()) return c;

    // creamos rutas aleatorias (entre 1 y maxLines)
    int numLines = 1 + rand() % m_maxLines;
    
    for (int i = 0; i < numLines; ++i) {
        CLine line(i + 1, sf::Color::Red); // el color se puede asignar despues
        int numStations = 2 + rand() % (m_stations.size() - 1); // minimo 2 estaciones
        
        // armamos la ruta con estaciones al azar (simulando un camino)
        int currentId = m_stations[rand() % m_stations.size()]->GetId();
        line.AddStation(currentId);
        
        for (int j = 0; j < numStations; ++j) {
            int nextId = m_stations[rand() % m_stations.size()]->GetId();
            if (!line.HasStation(nextId)) {
                line.AddStation(nextId);
            }
        }
        c.lines.push_back(line);
    }
    return c;
}

float GeneticAlgorithm::EvaluateChromosome(Chromosome& c) {
    // 1. Castigo por usar muchas rutas (q use las menos posibles)
    float penaltyLines = c.lines.size() * 500.0f; // si crea muchas lineas, el fitness sube (es peor)

    // 2. Revisamos cuantos puentes usa creando un grafo temporal
    CGraph<StationTrait<int>> graph(m_stations.size() + 1);
    for (auto* st : m_stations) {
        graph.AddStation(st->GetId(), st->GetGridX(), st->GetGridY(), st->GetPixelPos(), st->GetType());
    }

    int bridgesUsed = 0;
    for (auto& line : c.lines) {
        auto& stationsInLine = line.GetStations();
        
        // Evitamos el error si la linea esta vacia o solo tiene 1 estasion
        if (stationsInLine.size() < 2) continue; 

        for (size_t i = 0; i < stationsInLine.size() - 1; ++i) {
            auto* stA = graph.GetStationById(stationsInLine[i]);
            auto* stB = graph.GetStationById(stationsInLine[i+1]);
            if (!stA || !stB) continue;

            bool cruzaRio = m_grid->IsRiverCrossing(stA->GetPixelPos().x, stA->GetPixelPos().y, stB->GetPixelPos().x, stB->GetPixelPos().y);
            if (cruzaRio) {
                bridgesUsed++;
                if (bridgesUsed > m_maxBridges) return 9999999.0f; // castigo brutal
            }
            graph.AddEdge(stA->GetId(), stB->GetId(), cruzaRio);
        }
    }

    // 3. Evaluamos el tiempo de viaje real (distancia + 1 seg de espera por estacion)
    float totalTime = 0.0f;
    float speed = 150.0f; // velocidad constante en pixeles/seg

    for (auto& demand : m_demands) {
        std::vector<int> path = graph.FindShortestPathToType(demand.first, demand.second);
        if (path.empty()) {
            totalTime += 5000.0f; // castigo si no se puede llegar
        } else {
            // calculamos la distancia real recorrida
            for (size_t i = 0; i < path.size() - 1; ++i) {
                auto* stA = graph.GetStationById(path[i]);
                auto* stB = graph.GetStationById(path[i+1]);
                float dx = stA->GetPixelPos().x - stB->GetPixelPos().x;
                float dy = stA->GetPixelPos().y - stB->GetPixelPos().y;
                float dist = std::sqrt(dx*dx + dy*dy);
                totalTime += dist / speed; // tiempo de viaje
                totalTime += 1.0f; // 1 segundo de espera en la estacion
            }
        }
    }

    return totalTime + penaltyLines;
}

void GeneticAlgorithm::Mutate(Chromosome& c) {
    if (c.lines.empty()) return;
    // mutacion: cambiar una estacion de una linea al azar
    int lineIdx = rand() % c.lines.size();
    auto& stations = const_cast<std::vector<int>&>(c.lines[lineIdx].GetStations());
    
    if (!stations.empty() && rand() % 2 == 0) {
        stations[rand() % stations.size()] = m_stations[rand() % m_stations.size()]->GetId();
    }
}

std::vector<CLine> GeneticAlgorithm::Optimize() {
    std::cout << "iniciando IA con " << m_populationSize << " redes...\n";
    std::vector<Chromosome> population(m_populationSize);
    for (int i = 0; i < m_populationSize; ++i) population[i] = CreateRandomChromosome();

    for (int gen = 0; gen < m_generations; ++gen) {
        std::vector<std::future<float>> futures;
        for (int i = 0; i < m_populationSize; ++i) {
            futures.push_back(std::async(std::launch::async, &GeneticAlgorithm::EvaluateChromosome, this, std::ref(population[i])));
        }
        for (int i = 0; i < m_populationSize; ++i) population[i].fitness = futures[i].get();

        std::sort(population.begin(), population.end(), [](const Chromosome& a, const Chromosome& b) { return a.fitness < b.fitness; });

        std::vector<Chromosome> newPop;
        for (int i = 0; i < 10 && i < m_populationSize; ++i) newPop.push_back(population[i]);

        while (static_cast<int>(newPop.size()) < m_populationSize) {
            int p1 = rand() % 10;
            int p2 = rand() % 10;
            Chromosome child = population[p1];
            
            // cruce basico: mezclar estaciones de las lineas del padre 2
            for (auto& line : population[p2].lines) {
                if (rand() % 2 == 0) child.lines.push_back(line);
            }
            
            // si el hijo tiene mas lineas de las permitidas, quitamos las sobrantes
            while (static_cast<int>(child.lines.size()) > m_maxLines) {
                child.lines.pop_back();
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