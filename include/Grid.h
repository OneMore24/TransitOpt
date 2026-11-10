#ifndef __GRID_H__
#define __GRID_H__

#include <SFML/Graphics.hpp>
#include <iostream>

//definimos los tipos de terreno usando un enum class para evitar colisiones
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
    size_t m_rows;       // filas
    size_t m_columns;    // columnas
    size_t m_cellSize;   // tamaño en pixeles
    CellType* m_pData;   // Arreglo 1D contiguo
    // Metodo interno para calcular el indice 1D a partir de coordenadas 2D
    inline size_t GetIndex(size_t x, size_t y) const {
        return (y * m_columns) + x;
    }

public:
    CGrid(size_t rows, size_t columns, size_t cellSize);
    virtual ~CGrid();

    // Setters y getters
    void SetCell(size_t x, size_t y, CellType type);
    CellType GetCell(size_t x, size_t y) const;
    
    size_t GetRows() const { return m_rows; }
    size_t GetColumns() const { return m_columns; }
    size_t GetCellSize() const { return m_cellSize; }

    // SFML Drawing
    void Draw(sf::RenderWindow& window) const;
};

#endif // __GRID_H__