#include "DrawContext.h"
#include <cstdint>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
namespace CMPUT350
{

    DrawContext::DrawContext(std::shared_ptr<sf::RenderWindow> mWindow, std::shared_ptr<sf::Font> font)
        : mWindow(mWindow), mFont(font) {}

    sf::Color DrawContext::ToSFColor(RGBColor c)
    {
        return sf::Color(c.r, c.g, c.b);
    }

    void DrawContext::DrawText(const std::string &text, int pixelSize, Point2D p, RGBColor c)
    {
        sf::Text textObj(*mFont);

        textObj.setString(text);
        textObj.setCharacterSize(pixelSize);
        textObj.setFillColor(ToSFColor(c));
        textObj.setPosition(sf::Vector2f(p.x, p.y));
        mWindow->draw(textObj);
    }

    void DrawContext::DrawCircle(Point2D p, float radius, RGBColor c)
    {
        sf::CircleShape shape(radius);
        shape.setFillColor(ToSFColor(c));
        shape.setPosition(sf::Vector2f(p.x - radius, p.y - radius)); // center at p
        mWindow->draw(shape);
    }

    void DrawContext::DrawRect(Rect r, RGBColor c)
    {
        sf::RectangleShape shape(sf::Vector2f(r.width, r.height));
        shape.setFillColor(ToSFColor(c));
        shape.setPosition(sf::Vector2f(r.topLeft.x, r.topLeft.y));
        mWindow->draw(shape);
    }

    // Drawing a line with width
    void DrawContext::DrawLine(Point2D from, Point2D to, float width, RGBColor c)
    {
        // Compute the difference
        sf::Vector2f dir(to.x - from.x, to.y - from.y);
        float length = from.Distance(to);

        // Centers the line because we draw from top-left corner
        sf::RectangleShape line(sf::Vector2f(length, width));
        line.setFillColor(ToSFColor(c));
        line.setOrigin(sf::Vector2f(0, width / 2.0f));
        line.setPosition(sf::Vector2f(from.x, from.y));

        // Compute rotation in degrees
        float angle = std::atan2(dir.y, dir.x) * 180.0f / 3.14159265f;
        line.setRotation(sf::degrees(angle)); // line.setRotation(angle);

        mWindow->draw(line);
    }

    void DrawContext::SetGameplayViewport()
    {
        // Set viewport to only the left 1024px (gameplay area)
        // This prevents game objects from rendering over the GUI
        sf::View gameplayView(sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(1024, 1024)));
        gameplayView.setViewport(sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1024.0f / 1280.0f, 1.0f)));
        mWindow->setView(gameplayView);
    }

    void DrawContext::SetFullViewport()
    {
        // Reset to full screen view for GUI rendering
        sf::View fullView(sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(1280, 1024)));
        fullView.setViewport(sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1.0f, 1.0f)));
        mWindow->setView(fullView);
    }

    void DrawContext::SetContextCenter(Point2D p)
    {
        // Get the window size
        sf::Vector2u windowSize = mWindow->getSize();

        // Create a new view centered at `p`
        sf::View view;
        view.setCenter(sf::Vector2f(p.x, p.y));
        view.setSize(sf::Vector2f(windowSize.x, windowSize.y));

        mWindow->setView(view);
    }

    void DrawContext::SetGameplayCenterAndSize(Point2D center, sf::Vector2f size)
    {
        // Maintain the gameplay viewport while adjusting world view center/size
        sf::View view(sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(1024, 1024)));
        view.setViewport(sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1024.0f / 1280.0f, 1.0f)));
        view.setCenter(sf::Vector2f(center.x, center.y));
        view.setSize(size);
        mWindow->setView(view);
    }

}
