#include "vertex.hpp"
#include "edge/edge.hpp"

void Vertex::moveWithEdges(const sf::Vector2f &pos)
{
    sf::Vector2f delta = pos - position;
    position = pos;

    if (prevEdge) prevEdge->onEndpointMoved(this, delta);
    if (nextEdge) nextEdge->onEndpointMoved(this, delta);
}
