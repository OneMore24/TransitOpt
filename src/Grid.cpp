#include "Grid.h"

template <typename Traits>
CGrid<Traits>::CGrid(size_t rows, size_t columns, size_t cellSize) 
    : m_rows(rows), m_columns(columns), m_cellSize(cellSize) {
    
    // Reservamos memoria para el arreglo 1D
    m_pData = new CellType[m_rows * m_columns];

    // Inicializamos todo como Tierra
    for (size_t i = 0; i < m_rows * m_columns; ++i) {
        m_pData[i] = CellType::TIERRA;
    }

    // Creamos un río de prueba en la fila 7 (índice 7)
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
    return CellType::TIERRA; // Retorno por defecto si se sale de los límites
}

template <typename Traits>
void CGrid<Traits>::Draw(sf::RenderWindow& window) const {
    sf::RectangleShape rect(sf::Vector2f(static_cast<float>(m_cellSize), static_cast<float>(m_cellSize)));

    // Dibujamos el fondo
    for (size_t y = 0; y < m_rows; ++y) {
        for (size_t x = 0; x < m_columns; ++x) {
            // Corrección 1: Usar sf::Vector2f para setPosition
            rect.setPosition(sf::Vector2f(static_cast<float>(x * m_cellSize), static_cast<float>(y * m_cellSize)));
            
            if (m_pData[GetIndex(x, y)] == CellType::AGUA) {
                rect.setFillColor(sf::Color(147, 197, 253)); // Azul claro
            } else {
                rect.setFillColor(sf::Color(248, 250, 252)); // Blanco roto
            }
            window.draw(rect);
        }
    }

    // Corrección 2: Usar sf::PrimitiveType::Lines
    sf::VertexArray lineas(sf::PrimitiveType::Lines);
    
    for (size_t i = 0; i <= m_rows; ++i) {
        // Corrección 3: Usar llaves {} en lugar de paréntesis () para sf::Vertex
        lineas.append(sf::Vertex{sf::Vector2f(0, static_cast<float>(i * m_cellSize)), sf::Color(203, 213, 225)});
        lineas.append(sf::Vertex{sf::Vector2f(static_cast<float>(m_columns * m_cellSize), static_cast<float>(i * m_cellSize)), sf::Color(203, 213, 225)});
    }
    for (size_t j = 0; j <= m_columns; ++j) {
        lineas.append(sf::Vertex{sf::Vector2f(static_cast<float>(j * m_cellSize), 0), sf::Color(203, 213, 225)});
        lineas.append(sf::Vertex{sf::Vector2f(static_cast<float>(j * m_cellSize), static_cast<float>(m_rows * m_cellSize)), sf::Color(203, 213, 225)});
    }
    window.draw(lineas);
}

// Como usamos Templates, debemos instanciar la clase explícitamente al final
template class CGrid<GridTrait<TerrainType>>;