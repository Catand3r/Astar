#pragma once
#include "SFML/Graphics.hpp"
#include "astar.h"
#include <algorithm>
#include <vector>

class GraphicPathfinder
{
  private:
    const int cellSize_ = 50;

    sf::RenderWindow window_;

    std::vector<std::vector<sf::RectangleShape>> board_;

    Pathfinder::Pos startPos_ = Pathfinder::InvalidPos;

    Pathfinder::Pos endPos_ = Pathfinder::InvalidPos;

    std::vector<Pathfinder::Pos> forbiddenPos_;

    void updateBoard()
    {
        for (int x = 0; x < board_.size(); x++)
        {
            for (int y = 0; y < board_[x].size(); y++)
            {
                Pathfinder::Pos pos{x, y};
                if (pos == startPos_)
                    board_[x][y] = createCell(pos, sf::Color::Green);
                else if (pos == endPos_)
                    board_[x][y] = createCell(pos, sf::Color::Red);
                else if (std::find(forbiddenPos_.begin(), forbiddenPos_.end(), pos) != forbiddenPos_.end())
                    board_[x][y] = createCell(pos, sf::Color::Magenta);
                else
                    board_[x][y] = createCell(pos, sf::Color::White);
            }
        }
    }

    sf::RectangleShape createCell(Pathfinder::Pos pos, sf::Color color = sf::Color::White)
    {
        sf::RectangleShape rectangle;
        rectangle.setSize(sf::Vector2f(cellSize_, cellSize_));
        rectangle.setPosition(sf::Vector2f(pos.x_ * cellSize_, pos.y_ * cellSize_));
        rectangle.setOutlineThickness(-5.f);
        rectangle.setOutlineColor(sf::Color::Black);
        rectangle.setFillColor(color);
        return rectangle;
    }

    Pathfinder::Pos getSelectedCell()
    {
        sf::Vector2i mousePosition = sf::Mouse::getPosition(window_);

        for (int x = 0; x < board_.size(); x++)
        {
            for (int y = 0; y < board_.size(); y++)
            {
                if (board_[x][y].getGlobalBounds().contains(sf::Vector2f(mousePosition)))
                    return {x, y};
            }
        }

        return Pathfinder::InvalidPos;
    }

    void setStartPos(Pathfinder::Pos pos)
    {
        if (pos == Pathfinder::InvalidPos)
            return;
        startPos_ = pos;
        updateBoard();
    }

    void setEndPos(Pathfinder::Pos pos)
    {
        if (pos == Pathfinder::InvalidPos)
            return;
        endPos_ = pos;
        updateBoard();
    }

    void editForbiddenPos(Pathfinder::Pos pos)
    {
        if (pos == Pathfinder::InvalidPos)
            return;

        auto it = std::find(forbiddenPos_.begin(), forbiddenPos_.end(), pos);

        if (it == forbiddenPos_.end())
            forbiddenPos_.push_back(pos);
        else
            forbiddenPos_.erase(it);
        updateBoard();
    }

    void calculatePath()
    {
        Pathfinder pathfinder(startPos_, endPos_, forbiddenPos_, board_.size(), board_[0].size());
        std::vector<Pathfinder::Pos> path = pathfinder.Run();

        for (auto [x, y] : path)
        {
            if (Pathfinder::Pos{x, y} == startPos_ || Pathfinder::Pos{x, y} == endPos_)
                continue;

            board_[x][y].setFillColor(sf::Color::Blue);
        }
    }

  public:
    GraphicPathfinder(int boardSizeX, int boardSizeY)
        : window_(sf::VideoMode(sf::Vector2u{static_cast<unsigned int>(boardSizeX * cellSize_),
                                             static_cast<unsigned int>(boardSizeY * cellSize_)}),
                  "A*")
    {
        board_.resize(boardSizeX);
        for (int i = 0; i < board_.size(); i++)
        {
            board_[i].resize(boardSizeY);
        }

        updateBoard();
    }

    void Draw()
    {
        while (window_.isOpen())
        {
            while (const std::optional event = window_.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                    window_.close();
                else if (event->is<sf::Event::KeyPressed>())
                {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                        setStartPos(getSelectedCell());
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
                        setEndPos(getSelectedCell());
                    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter))
                        calculatePath();
                }
                else if (event->is<sf::Event::MouseButtonPressed>())
                {
                    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
                        editForbiddenPos(getSelectedCell());
                }
            }

            window_.clear(sf::Color::Black);

            for (int i = 0; i < board_.size(); i++)
            {
                for (auto &cell : board_[i])
                {
                    window_.draw(cell);
                }
            }

            window_.display();
        }
    }
};
