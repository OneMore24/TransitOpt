#ifndef __GRID_H__
#define __GRID_H__

#include <SFML/Graphics.hpp>
#include <iostream>

enum class TerrainType {
    TIERRA = 0,
    AGUA   = 1
};

template <typename _T>
struct GridTrait {
    using CellType = _T;
};

template <typename Traits>
class CGrid {
    using CellType = typename Traits::CellType;

private:
    size_t m_rows;
    size_t m_columns;
    size_t m_cellSize;
    CellType* m_pData; 

    inline size_t GetIndex(size_t x, size_t y) const {
        return (y * m_columns) + x;
    }

public:
    CGrid(size_t rows, size_t columns, size_t cellSize);
    virtual ~CGrid();

    void SetCell(size_t x, size_t y, CellType type);
    CellType GetCell(size_t x, size_t y) const;
    
    size_t GetRows() const { return m_rows; }
    size_t GetColumns() const { return m_columns; }
    size_t GetCellSize() const { return m_cellSize; }

    sf::Vector2u GetGridCoords(float pixelX, float pixelY) const;
    bool IsRiverCrossing(float x1, float y1, float x2, float y2) const;

    // añadimos el offsetX pa saber desde donde empiesa a dibujar
    void Draw(sf::RenderWindow& window, float offsetX = 0.0f) const;
};

#endif // __GRID_H__