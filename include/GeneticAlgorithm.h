#ifndef __GENETIC_ALGORITHM_H__
#define __GENETIC_ALGORITHM_H__

#include <vector>
#include <random>
#include <future>
#include <iostream>
#include <thread>
#include <atomic>
#include <unordered_map>
#include "Graph.h"
#include "Grid.h"
#include "Station.h"
#include "Line.h"

struct Chromosome {
    std::vector<CLine> lines;
    float fitness = 999999999.0f; // inicializado por defecto
};

class GeneticAlgorithm {
private:
    std::vector<CStation<StationTrait<int>>*> m_stations;
    const CGrid<GridTrait<TerrainType>>* m_grid;
    int m_maxBridges;
    int m_maxLines;
    int m_populationSize;
    int m_generations;

    std::atomic<int> m_currentGen;
    std::atomic<float> m_bestFitness;

    // matrices precalculadas pa aselerar la evaluacion
    std::vector<std::vector<float>> m_distMatrix;
    std::vector<std::vector<bool>> m_riverMatrix;
    std::unordered_map<int, size_t> m_idToIndex; // mapea ID de estacion -> indice del vector

    Chromosome CreateRandomChromosome();
    float EvaluateChromosome(Chromosome& c);
    void Mutate(Chromosome& c);
    Chromosome Crossover(const Chromosome& p1, const Chromosome& p2);

public:
    GeneticAlgorithm(const std::vector<CStation<StationTrait<int>>*>& stations, 
                     const CGrid<GridTrait<TerrainType>>& grid, 
                     int maxBridges, int maxLines,
                     int popSize, int gens);
                     
    std::vector<CLine> Optimize();

    int GetCurrentGeneration() const { return m_currentGen.load(); }
    float GetBestFitness() const { return m_bestFitness.load(); }
};

#endif