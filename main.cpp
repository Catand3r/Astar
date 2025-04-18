#include "astar.h"
#include "astarSFML.h"

int main()
{
    // Pathfinder::Pos startPos = {2, 3};
    // Pathfinder::Pos endPos = {5, 4};
    // std::vector<Pathfinder::Pos> forbiddenPos{{4, 4}, {4, 3}, {4, 5}, {5, 3}};
    // Pathfinder pathfinder(startPos, endPos, forbiddenPos, 6, 6);
    // pathfinder.Run();

    GraphicPathfinder graphicPathfinder(10, 10);

    graphicPathfinder.Draw();
}
