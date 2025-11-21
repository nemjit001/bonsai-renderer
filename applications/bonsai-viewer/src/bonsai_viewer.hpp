#pragma once
#ifndef BONSAI_RENDERER_BONSAI_VIEWER_HPP
#define BONSAI_RENDERER_BONSAI_VIEWER_HPP

#include <bonsai/application.hpp>

class BONSAI_API BonsaiViewer : public Application
{
public:
    BonsaiViewer() = default;
    ~BonsaiViewer() override = default;

    void update(double delta) override;

    char const* name() const override { return "Bonsai Viewer"; }
};

#endif //BONSAI_RENDERER_BONSAI_VIEWER_HPP