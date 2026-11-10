#include "Grid.h"

template <typename Traits>
CGrid<Traits>::CGrid(size_t rows, size_t columns, size_t cellSize) 
    : m_rows(rows), m_columns(columns), m_cellSize(cellSize) {
    
    m_pData = new CellType[m_rows * m_columns];

    // todo empiesa como tierra, el usuario ya pintara el rio
    for (size_t i = 0; i < m_rows * m_columns; ++i) {
        m_pData[i] = CellType::TIERRA; 
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
    return CellType::TIERRA; 
}

template <typename Traits>
sf::Vector2u CGrid<Traits>::GetGridCoords(float pixelX, float pixelY) const {
    size_t gridX = static_cast<size_t>(pixelX / m_cellSize);
    size_t gridY = static_cast<size_t>(pixelY / m_cellSize);
    return sf::Vector2u(gridX, gridY);
}

template <typename Traits>
void CGrid<Traits>::Draw(sf::RenderWindow& window, float offsetX) const {
    sf::RectangleShape rect(sf::Vector2f(static_cast<float>(m_cellSize), static_cast<float>(m_cellSize)));

    for (size_t y = 0; y < m_rows; ++y) {
        for (size_t x = 0; x < m_columns; ++x) {
            // sumamos el offsetX a la posision
            rect.setPosition(sf::Vector2f(static_cast<float>(x * m_cellSize) + offsetX, static_cast<float>(y * m_cellSize)));
            
            if (m_pData[GetIndex(x, y)] == CellType::AGUA) {
                rect.setFillColor(sf::Color(147, 197, 253)); 
            } else {
                rect.setFillColor(sf::Color(248, 250, 252)); 
            }
            window.draw(rect);
        }
    }

    sf::VertexArray lineas(sf::PrimitiveType::Lines);
    
    for (size_t i = 0; i <= m_rows; ++i) {
        lineas.append(sf::Vertex{sf::Vector2f(offsetX, static_cast<float>(i * m_cellSize)), sf::Color(203, 213, 225)});
        lineas.append(sf::Vertex{sf::Vector2f(offsetX + static_cast<float>(m_columns * m_cellSize), static_cast<float>(i * m_cellSize)), sf::Color(203, 213, 225)});
    }
    for (size_t j = 0; j <= m_columns; ++j) {
        lineas.append(sf::Vertex{sf::Vector2f(offsetX + static_cast<float>(j * m_cellSize), 0), sf::Color(203, 213, 225)});
        lineas.append(sf::Vertex{sf::Vector2f(offsetX + static_cast<float>(j * m_cellSize), static_cast<float>(m_rows * m_cellSize)), sf::Color(203, 213, 225)});
    }
    window.draw(lineas);
}

template <typename Traits>
bool CGrid<Traits>::IsRiverCrossing(float x1, float y1, float x2, float y2) const {
    // pasamos de pixeles a cordenadas de la grid (usamos int, no size_t, pa evitar el underflow)
    int gx1 = static_cast<int>(x1 / m_cellSize);
    int gy1 = static_cast<int>(y1 / m_cellSize);
    int gx2 = static_cast<int>(x2 / m_cellSize);
    int gy2 = static_cast<int>(y2 / m_cellSize);

    // Algoritmo de Bresenham: es la forma mas rapida de trazar una linea en una cuadricula
    int dx = std::abs(gx2 - gx1);
    int sx = (gx1 < gx2) ? 1 : -1;
    int dy = -std::abs(gy2 - gy1);
    int sy = (gy1 < gy2) ? 1 : -1;
    int err = dx + dy; 

    while (true) {
        // si la celda actual es agua, ya cruzamos el rio
        if (GetCell(gx1, gy1) == TerrainType::AGUA) {
            return true;
        }
        
        // si llegamos al destino, terminamos
        if (gx1 == gx2 && gy1 == gy2) break;

        int e2 = 2 * err;
        if (e2 >= dy) { 
            err += dy; 
            gx1 += sx; 
        }
        if (e2 <= dx) { 
            err += dx; 
            gy1 += sy; 
        }
    }
    return false;
}

template class CGrid<GridTrait<TerrainType>>;