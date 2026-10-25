#pragma once
#include <memory>
#include "../enum/continuity.hpp"

class Vertex;
class Edge;

class ContinuityStrategy {
public:
    virtual ~ContinuityStrategy() = default;
    virtual Continuity getType() const = 0;

    virtual void enforce(Vertex* vertex, Edge* prevEdge, Edge* nextEdge, bool movedFromPrev) const = 0;

    static std::unique_ptr<ContinuityStrategy> create(Continuity type);
};

class G0Strategy : public ContinuityStrategy {
public:
    Continuity getType() const override { return Continuity::G0; }
    void enforce(Vertex*, Edge*, Edge*, bool) const override;
};

class G1Strategy : public ContinuityStrategy {
public:
    Continuity getType() const override { return Continuity::G1; }
    void enforce(Vertex* vertex, Edge* prevEdge, Edge* nextEdge, bool movedFromPrev) const override;
};

class C1Strategy : public ContinuityStrategy {
public:
    Continuity getType() const override { return Continuity::C1; }
    void enforce(Vertex* vertex, Edge* prevEdge, Edge* nextEdge, bool movedFromPrev) const override;
};

inline std::unique_ptr<ContinuityStrategy> ContinuityStrategy::create(Continuity type) {
    switch (type) {
        case Continuity::G0: return std::make_unique<G0Strategy>();
        case Continuity::G1: return std::make_unique<G1Strategy>();
        case Continuity::C1: return std::make_unique<C1Strategy>();
    }
    return std::make_unique<G0Strategy>();
}