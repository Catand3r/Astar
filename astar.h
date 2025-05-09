#pragma once
#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

class Pathfinder
{
  public:
    struct Pos
    {
        Pos(int x, int y) : x_(x), y_(y)
        {
        }

        int x_ = 0;
        int y_ = 0;

        Pos operator+(const Pos &pos)
        {
            return Pos(x_ + pos.x_, y_ + pos.y_);
        }

        Pos operator-(const Pos &pos)
        {
            return Pos(x_ - pos.x_, y_ - pos.y_);
        }

        bool operator==(const Pos &pos) const
        {
            return x_ == pos.x_ && y_ == pos.y_;
        }

        bool operator!=(const Pos &pos) const
        {
            return x_ != pos.x_ || y_ != pos.y_;
        }

        bool isValid(const std::vector<Pos> &forbiddenPos, int boardSizeX, int boardSizeY) const
        {
            for (auto elem : forbiddenPos)
            {
                if (*this == elem)
                    return false;
            }

            return x_ >= 0 && y_ >= 0 && x_ < boardSizeX && y_ < boardSizeY;
        }

        float calcDistance(const Pos &pos)
        {
            return std::abs(x_ - pos.x_) + std::abs(y_ - pos.y_);
        }
    };

  private:
    struct Node
    {
        Node() = default;

        Node(Node *parent, Pos pos, Pos endPos) : parentPos_(parent ? parent->pos_ : Pos{-1, -1}), pos_(pos)
        {
            if (parent)
            {
                g = parent->g + pos_.calcDistance(parentPos_);
                h = pos_.calcDistance(endPos);
                f = g + h;
            }
        }

        Pos parentPos_ = {-1, -1};

        bool isWall_ = false;
        Pos pos_ = {0, 0};

        float f = 0.0f;
        float g = 0.0f; // koszt kroku
        float h = 0.0f; // koszt dojœcia do celu
    };

    std::vector<std::vector<Node>> board_;

    std::vector<Node> open_;
    std::vector<Node> closed_;

    std::vector<Pos> path_;

    std::vector<Pos> forbiddenPos_;

    const Pos startPos_ = {0, 0};
    const Pos endPos_ = {0, 0};

    bool shouldSkipSuccessor(const std::vector<Node> &vect, const Node &successor) const
    {
        return vect.end() != std::find_if(vect.begin(), vect.end(), [&successor](const Node &node) {
                   return successor.pos_ == node.pos_ && successor.f >= node.f;
               });
    }

  public:
    Pathfinder(Pos startPos, Pos endPos, std::vector<Pos> forbiddenPos, unsigned int boardSizeX,
               unsigned int boardSizeY)
        : startPos_(startPos), endPos_(endPos), forbiddenPos_(forbiddenPos)
    {
        board_.resize(boardSizeX);
        for (int i = 0; i < boardSizeX; i++)
        {
            board_[i].resize(boardSizeY);
            for (int j = 0; j < boardSizeY; j++)
            {
                Node node(nullptr, {i, j}, endPos_);
                board_[i][j] = node;
            }
        }
        Node node(nullptr, startPos_, endPos_);
        open_.push_back(node);
    }

    inline static const Pos InvalidPos = {-1, -1};

    bool Run()
    {
        if (open_.empty())
            return {};

        Node q = open_.back();
        open_.pop_back();
        std::cout << "Node {" << q.pos_.x_ << ", " << q.pos_.y_ << "} start processing; parentNode {" << q.parentPos_.x_
                  << ", " << q.parentPos_.y_ << "}\n";

        auto steps = {Pos(0, -1), Pos(1, 1), Pos(1, 0), Pos(1, -1), Pos(0, 1), Pos(-1, -1), Pos(-1, 0), Pos(-1, 1)};
        for (auto pos : steps)
        {
            Pos nodePos(q.pos_ + pos);

            if (!nodePos.isValid(forbiddenPos_, board_.size(), board_[0].size()))
                continue;
            if (nodePos == endPos_)
            {
                Node currentNode(&q, nodePos, endPos_);

                path_.push_back(endPos_);

                while (currentNode.pos_ != startPos_)
                {
                    auto [parentX, parentY] = currentNode.parentPos_;
                    currentNode = board_[parentX][parentY];
                    path_.push_back(currentNode.pos_);
                }

                for (const auto &elem : path_)
                {
                    std::cout << "Path {" << elem.x_ << ", " << elem.y_ << "}\n";
                }
                return true;
            }
            Node successor(&q, nodePos, endPos_);

            if (shouldSkipSuccessor(open_, successor) || shouldSkipSuccessor(closed_, successor))
            {
                std::cout << "Node {" << successor.pos_.x_ << ", " << successor.pos_.y_ << "} will be reject\n";
                continue;
            }

            board_[nodePos.x_][nodePos.y_] = successor;

            std::cout << "Node {" << successor.pos_.x_ << ", " << successor.pos_.y_ << "} will be push back\n";
            open_.push_back(successor);
        }

        std::sort(open_.begin(), open_.end(), [](const Node &node1, const Node &node2) { return node1.f > node2.f; });

        closed_.push_back(q);

        return false;

        // std::cout << "Nie znaleziono sciezki do celu\n";
        // return {};
    }

    std::vector<Pos> GetClosedList() const
    {
        std::vector<Pos> closedList;

        for (const auto &elem : closed_)
        {
            closedList.push_back(elem.pos_);
        }
        return closedList;
    }

    std::vector<Pos> GetOpenList() const
    {
        std::vector<Pos> openList;

        for (const auto &elem : open_)
        {
            openList.push_back(elem.pos_);
        }
        return openList;
    }

    const std::vector<Pos> &GetPath() const
    {
        return path_;
    }
};

/*
drukowanie odleglosci do endposa na cellach (f)
*/
