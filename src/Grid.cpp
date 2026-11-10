#include "Grid.h"

template <typename Traits>
CGrid<Traits>::CGrid(size_t rows, size_t columns, size_t cellSize) 
    : m_rows(rows), m_columns(columns), m_cellSize(cellSize) {
    
    // reservamos memoria pa la grid
    m_pData = new CellType[m_rows * m_columns];

    for (size_t i = 0; i < m_rows * m_columns; ++i) {
        m_pData[i] = CellType::TIERRA; // todo empiesa como tierra
    }

    // creamos un rio de prueba en la fila 7
    for (size_t x = 0; x < m_columns; ++x) {
        SetCell(x, 7, CellType::AGUA);
    }
}

template <typename Traits>
CGrid<Traits>::~CGrid() {
    if (m_pData) {
        delete[] m_pData;
        m_pData = nullptr;
    }
}

template <typename Traits>
void CGrid<Traits>::SetCell(size_t x, size_t y, CellType type) {
    if (x < m_columns && y < m_rows) {
        m_pData[GetIndex(x, y)] = type;
    }
}

template <typename Traits>
typename CGrid<Traits>::CellType CGrid<Traits>::GetCell(size_t x, size_t y) const {
    if (x < m_columns && y < m_rows) {
        return m_pData[GetIndex(x, y)];
    }
    return CellType::TIERRA; // retorno por defecto
}

template <typename Traits>
sf::Vector2u CGrid<Traits>::GetGridCoords(float pixelX, float pixelY) const {
    // dividimos el pixel entre el tamaño de la celda
    size_t gridX = static_cast<size_t>(pixelX / m_cellSize);
    size_t gridY = static_cast<size_t>(pixelY / m_cellSize);
    return sf::Vector2u(gridX, gridY);
}

template <typename Traits>
void CGrid<Traits>::Draw(sf::RenderWindow& window) const {
    sf::RectangleShape rect(sf::Vector2f(static_cast<float>(m_cellSize), static_cast<float>(m_cellSize)));

    for (size_t y = 0; y < m_rows; ++y) {
        for (size_t x = 0; x < m_columns; ++x) {
            rect.setPosition(sf::Vector2f(static_cast<float>(x * m_cellSize), static_cast<float>(y * m_cellSize)));
            
            if (m_pData[GetIndex(x, y)] == CellType::AGUA) {
                rect.setFillColor(sf::Color(147, 197, 253)); // azul
            } else {
                rect.setFillColor(sf::Color(248, 250, 252)); // blanco
            }
            window.draw(rect);
        }
    }

    // dibujamos las lineas de la grid
    sf::VertexArray lineas(sf::PrimitiveType::Lines);
    
    for (size_t i = 0; i <= m_rows; ++i) {
        lineas.append(sf::Vertex{sf::Vector2f(0, static_cast<float>(i * m_cellSize)), sf::Color(203, 213, 225)});
        lineas.append(sf::Vertex{sf::Vector2f(static_cast<float>(m_columns * m_cellSize), static_cast<float>(i * m_cellSize)), sf::Color(203, 213, 225)});
    }
    for (size_t j = 0; j <= m_columns; ++j) {
        lineas.append(sf::Vertex{sf::Vector2f(static_cast<float>(j * m_cellSize), 0), sf::Color(203, 213, 225)});
        lineas.append(sf::Vertex{sf::Vector2f(static_cast<float>(j * m_cellSize), static_cast<float>(m_rows * m_cellSize)), sf::Color(203, 213, 225)});
    }
    window.draw(lineas);
}

template class CGrid<GridTrait<TerrainType>>;