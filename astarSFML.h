#pragma once
#include "SFML/Graphics.hpp"
#include "astar.h"
#include <algorithm>
#include <mutex>
#include <thread>
#include <vector>

class GraphicPathfinder
{
  private:
    enum class States
    {
        input = 0,
        calculating
    };

    States state_ = States::input;

    std::thread calculateThread;

    std::atomic_bool calculateThreadRunning = false;

    std::mutex boardMutex_;

    const int cellSize_ = 50;

    sf::RenderWindow window_;

    std::vector<std::vector<sf::RectangleShape>> board_;

    Pathfinder::Pos startPos_ = Pathfinder::InvalidPos;

    Pathfinder::Pos endPos_ = Pathfinder::InvalidPos;

    std::vector<Pathfinder::Pos> forbiddenPos_;

    Pathfinder *pathfinder_ = nullptr;

    sf::Clock clock_;

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
                    board_[x][y] = createCell(pos, sf::Color::Cyan);
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

    void setCellColor(Pathfinder::Pos pos, sf::Color color)
    {
        std::scoped_lock sl(boardMutex_);
        board_[pos.x_][pos.y_].setFillColor(color);
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

    bool calculatePath()
    {
        bool reachDestination = pathfinder_->Run();
        std::cout << "Is endPos reached: " << reachDestination << "\n";

        // std::mutex - blokada przed dostepem do zasobu wspolnego
        if (reachDestination)
        {
            auto &path = pathfinder_->GetPath();
            for (const auto &[x, y] : path)
            {
                board_[x][y].setFillColor(sf::Color::Blue);
            }
        }
        else
        {
            auto open = pathfinder_->GetOpenList();
            std::cout << "openList size = " << open.size() << '\n';
            for (const auto &[x, y] : open)
            {
                board_[x][y].setFillColor(sf::Color::Magenta);
                // pathFinder.GetEstimatedDistance( x, y);
            }

            auto closed = pathfinder_->GetClosedList();
            std::cout << "closedList size = " << closed.size() << '\n';
            for (const auto &[x, y] : closed)
            {
                board_[x][y].setFillColor(sf::Color::Yellow);
            }
        }
        board_[startPos_.x_][startPos_.y_].setFillColor(sf::Color::Green);
        board_[endPos_.x_][endPos_.y_].setFillColor(sf::Color::Red);

        return reachDestination;
    }

    bool calculatePath2()
    {
        // std::mutex - blokada przed dostepem do zasobu wspolnego
        while (!pathfinder_->GetOpenList().empty())
        {
            sf::sleep(sf::milliseconds(500));
            if (pathfinder_->Run())
            {
                auto &path = pathfinder_->GetPath();
                for (const auto &[x, y] : path)
                {
                    setCellColor({x, y}, sf::Color::Blue);
                }
                calculateThreadRunning = false;
                return true;
            }
            else
            {
                auto open = pathfinder_->GetOpenList();
                std::cout << "openList size = " << open.size() << '\n';
                for (const auto &[x, y] : open)
                {
                    setCellColor({x, y}, sf::Color::Magenta);
                    // pathFinder.GetEstimatedDistance( x, y);
                }

                auto closed = pathfinder_->GetClosedList();
                std::cout << "closedList size = " << closed.size() << '\n';
                for (const auto &[x, y] : closed)
                {
                    setCellColor({x, y}, sf::Color::Yellow);
                }
            }
            setCellColor({startPos_.x_, startPos_.y_}, sf::Color::Green);
            setCellColor({endPos_.x_, endPos_.y_}, sf::Color::Red);
        }
        calculateThreadRunning = false;
        return false;
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

    ~GraphicPathfinder()
    {
        if (calculateThread.joinable())
            calculateThread.join();
    }

    void Draw()
    {
        while (window_.isOpen())
        {
            while (const std::optional event = window_.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                    window_.close();
                if (state_ == States::input)
                {
                    if (event->is<sf::Event::KeyPressed>())
                    {
                        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                            setStartPos(getSelectedCell());
                        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
                            setEndPos(getSelectedCell());
                        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter))
                        {
                            pathfinder_ =
                                new Pathfinder(startPos_, endPos_, forbiddenPos_, board_.size(), board_[0].size());
                            state_ = States::calculating;

                            calculateThread = std::thread(&GraphicPathfinder::calculatePath2, this);
                            calculateThreadRunning = true;
                        }
                    }
                    else if (event->is<sf::Event::MouseButtonPressed>())
                    {
                        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
                            editForbiddenPos(getSelectedCell());
                    }
                }
            }

            if (state_ == States::calculating)
            {
                if (!calculateThreadRunning)
                    state_ = States::input;
            }

            window_.clear(sf::Color::Black);

            {
                // pocz¹tek sekcji krytycznej
                std::scoped_lock sl(boardMutex_);
                for (int i = 0; i < board_.size(); i++)
                {
                    for (auto &cell : board_[i])
                    {
                        window_.draw(cell);
                    }
                }
                // koniec sekcji krytycznej
            }
            window_.display();
        }
    }
};
