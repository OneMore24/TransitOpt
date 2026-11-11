#ifndef __GENETIC_ALGORITHM_H__
#define __GENETIC_ALGORITHM_H__

#include <vector>
#include <random>
#include <future>
#include <iostream>
#include <thread>
#include "Graph.h"
#include "Grid.h"
#include "Station.h"
#include "Line.h"

struct Chromosome {
    std::vector<CLine> lines;
    float fitness;
};

class GeneticAlgorithm {
private:
    std::vector<CStation<StationTrait<int>>*> m_stations;
    const CGrid<GridTrait<TerrainType>>* m_grid;
    int m_maxBridges;
    int m_maxLines;
    int m_populationSize;
    int m_generations;

    Chromosome CreateRandomChromosome();
    float EvaluateChromosome(Chromosome& c);
    void Mutate(Chromosome& c);

public:
    GeneticAlgorithm(const std::vector<CStation<StationTrait<int>>*>& stations, 
                     const CGrid<GridTrait<TerrainType>>& grid, 
                     int maxBridges, int maxLines,
                     int popSize, int gens);
                     
    std::vector<CLine> Optimize();
};

#endif