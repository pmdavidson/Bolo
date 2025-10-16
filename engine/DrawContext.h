#ifndef DRAWCONTEXT_H
#define DRAWCONTEXT_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdint>
#include "MathUtil.h"

namespace CMPUT350
{

    struct RGBColor
    {
        uint8_t r, g, b;
        RGBColor(uint8_t r, uint8_t g, uint8_t b)
            : r(r), g(g), b(b) {}
    };

    namespace Colors
    {
        const RGBColor red(255, 0, 0);
        const RGBColor green(0, 255, 0);
        const RGBColor blue(0, 0, 255);
        const RGBColor yellow(255, 255, 0);
        const RGBColor cyan(0, 255, 255);
        const RGBColor magenta(255, 0, 255);
        const RGBColor white(255, 255, 255);
        const RGBColor black(0, 0, 0);
        const RGBColor gray(100, 100, 100);
    }

    class DrawContext
    {
    public:
        DrawContext(std::shared_ptr<sf::RenderWindow> mWindow, std::shared_ptr<sf::Font> font);
        void DrawText(const std::string &text, int pixelSize, Point2D p, RGBColor c);
        void DrawCircle(Point2D p, float radius, RGBColor c);
        void DrawRect(Rect r, RGBColor c);
        void DrawLine(Point2D from, Point2D to, float width, RGBColor c);
        void SetGameplayViewport();
        void SetFullViewport();
        void SetContextCenter(Point2D p);

    private:
        std::shared_ptr<sf::RenderWindow> mWindow;
        std::shared_ptr<sf::Font> mFont;
        sf::Color ToSFColor(RGBColor c);
    };

}

#endif // DRAWCONTEXT_H
