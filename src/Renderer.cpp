#include "Renderer.h"
#include <cmath>

void Renderer::DrawStationShape(sf::RenderWindow& window, StationType type, sf::Vector2f pos, float size, sf::Color color) {
    sf::CircleShape circle(size);
    circle.setOrigin(sf::Vector2f(size, size));
    circle.setPosition(pos);
    circle.setFillColor(color);
    circle.setOutlineThickness(3);
    circle.setOutlineColor(sf::Color::Black);

    sf::RectangleShape rect(sf::Vector2f(size * 2, size * 2));
    rect.setOrigin(sf::Vector2f(size, size));
    rect.setPosition(pos);
    rect.setFillColor(color);
    rect.setOutlineThickness(3);
    rect.setOutlineColor(sf::Color::Black);

    switch (type) {
        case StationType::CIRCULO: circle.setPointCount(30); window.draw(circle); break;
        case StationType::CUADRADO: window.draw(rect); break;
        case StationType::TRIANGULO: circle.setPointCount(3); window.draw(circle); break;
        case StationType::OVALO: circle.setPointCount(30); circle.setScale(sf::Vector2f(1.5f, 0.7f)); window.draw(circle); break;
        case StationType::PENTAGONO: circle.setPointCount(5); window.draw(circle); break;
        case StationType::CONO: circle.setPointCount(3); circle.setScale(sf::Vector2f(0.8f, 1.5f)); window.draw(circle); break;
        case StationType::DIAMANTE: circle.setPointCount(4); circle.setRotation(sf::degrees(45)); window.draw(circle); break;
        case StationType::ESTRELLA: {
            sf::ConvexShape star; star.setPointCount(10);
            for(int i=0; i<10; i++) {
                float angle = i * 36.0f * 3.14159f / 180.0f;
                float r = (i % 2 == 0) ? size : size * 0.4f;
                star.setPoint(i, sf::Vector2f(r * std::cos(angle), r * std::sin(angle)));
            }
            star.setOrigin(sf::Vector2f(0,0)); star.setPosition(pos); 
            star.setFillColor(color); star.setOutlineThickness(3); star.setOutlineColor(sf::Color::Black);
            window.draw(star); break;
        }
        case StationType::RECTANGULO: rect.setScale(sf::Vector2f(1.5f, 0.8f)); window.draw(rect); break;
        case StationType::CRUZ: {
            sf::RectangleShape r1(sf::Vector2f(size * 2, size * 0.7f)); r1.setOrigin(sf::Vector2f(size, size*0.35f)); r1.setPosition(pos); r1.setFillColor(color); r1.setOutlineThickness(2); r1.setOutlineColor(sf::Color::Black);
            sf::RectangleShape r2(sf::Vector2f(size * 0.7f, size * 2)); r2.setOrigin(sf::Vector2f(size*0.35f, size)); r2.setPosition(pos); r2.setFillColor(color); r2.setOutlineThickness(2); r2.setOutlineColor(sf::Color::Black);
            window.draw(r1); window.draw(r2); break;
        }
    }
}

void Renderer::DrawMap(sf::RenderWindow& window, const CGrid<GridTrait<TerrainType>>& mapa, float offsetX) {
    mapa.Draw(window, offsetX);
}

void Renderer::DrawLines(sf::RenderWindow& window, const std::vector<CLine>& lineas, const CGraph<StationTrait<int>>& redMetro, const CGrid<GridTrait<TerrainType>>& mapa, float offsetX) {
    for (const auto& linea : lineas) {
        const auto& stationsInLine = linea.GetStations();
        
        if (stationsInLine.size() < 2) continue;

        for (size_t i = 0; i < stationsInLine.size() - 1; ++i) {
            auto* stA = redMetro.GetStationById(stationsInLine[i]);
            auto* stB = redMetro.GetStationById(stationsInLine[i+1]);
            if (!stA || !stB) continue;

            sf::Vector2f posA(stA->GetPixelPos().x + offsetX, stA->GetPixelPos().y);
            sf::Vector2f posB(stB->GetPixelPos().x + offsetX, stB->GetPixelPos().y);

            sf::Vector2f dir = posB - posA;
            float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (length == 0) continue;
            
            float angle = std::atan2(dir.y, dir.x) * 180.0f / 3.14159f;
            
            dir /= length; 
            sf::Vector2f perp(-dir.y, dir.x); 
            float offsetMagnitude = 5.0f * (linea.GetId() - 1); 
            sf::Vector2f offset = perp * offsetMagnitude;

            float grosorVia = 8.0f;
            sf::RectangleShape via(sf::Vector2f(length, grosorVia));
            via.setOrigin(sf::Vector2f(0, grosorVia / 2.0f));
            via.setPosition(posA + offset);
            via.setRotation(sf::degrees(angle));
            via.setFillColor(linea.GetColor());

            // revisamos si cruza el rio usando el mapa
            bool cruzaRio = mapa.IsRiverCrossing(stA->GetPixelPos().x, stA->GetPixelPos().y, stB->GetPixelPos().x, stB->GetPixelPos().y);
            
            if (cruzaRio) {
                // si cruza el rio, le ponemos un borde blanco grueso pa q parezca un puente/tunel
                via.setOutlineThickness(3.0f);
                via.setOutlineColor(sf::Color::White);
                // opcional: el relleno sea mas oscuro
                sf::Color c = linea.GetColor();
                via.setFillColor(sf::Color(c.r * 0.7, c.g * 0.7, c.b * 0.7));
            } else {
                // si no cruza, un borde sutil pa q resalte
                via.setOutlineThickness(1.0f);
                via.setOutlineColor(sf::Color(0, 0, 0, 100));
            }

            window.draw(via);
        }
    }
}

void Renderer::DrawStations(sf::RenderWindow& window, const CGraph<StationTrait<int>>& redMetro, int selectedId, float offsetX, int cellSize) {
    for (size_t i = 0; i < redMetro.GetNumStations(); ++i) {
        auto* station = redMetro.GetStation(i);
        sf::Color fill = (station->GetId() == selectedId) ? sf::Color::Green : sf::Color::White;
        sf::Vector2f drawPos(station->GetPixelPos().x + offsetX, station->GetPixelPos().y);
        DrawStationShape(window, station->GetType(), drawPos, cellSize / 3.0f, fill);
    }
}

void Renderer::DrawPassengers(sf::RenderWindow& window, const std::vector<CPassenger>& pasajeros, float offsetX) {
    for (const auto& p : pasajeros) {
        sf::Vector2f pPos = p.GetPixelPos();
        sf::CircleShape c(4); c.setOrigin(sf::Vector2f(4,4));
        c.setPosition(sf::Vector2f(pPos.x + offsetX, pPos.y));
        c.setFillColor(sf::Color::Red);
        window.draw(c);
    }
}

void Renderer::DrawMenu(sf::RenderWindow& window, const StationType tipos[10], int menuWidth, bool modoRio, sf::Color lineaActualColor) {
    sf::RectangleShape menuBg(sf::Vector2f(menuWidth, window.getSize().y));
    menuBg.setFillColor(modoRio ? sf::Color(100, 149, 237) : sf::Color(60, 60, 60));
    menuBg.setOutlineThickness(2);
    menuBg.setOutlineColor(sf::Color::Black);
    window.draw(menuBg);

    for (int i = 0; i < 10; ++i) {
        float posY = (i * window.getSize().y / 10.0f) + (window.getSize().y / 20.0f);
        sf::Vector2f pos(menuWidth / 2.0f, posY);
        DrawStationShape(window, tipos[i], pos, 15, sf::Color::White); 
    }
    
    // indicador de linea actual
    sf::RectangleShape lineIndicator(sf::Vector2f(menuWidth - 20, 30));
    lineIndicator.setPosition(sf::Vector2f(10, window.getSize().y - 40));
    lineIndicator.setFillColor(lineaActualColor);
    lineIndicator.setOutlineThickness(2);
    lineIndicator.setOutlineColor(sf::Color::White);
    window.draw(lineIndicator);
}