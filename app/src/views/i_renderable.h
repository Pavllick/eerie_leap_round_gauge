#pragma once

namespace eerie_leap::views {

class IRenderable {
protected:
    bool is_ready_ = false;

    virtual int DoRender() = 0;

public:
    virtual ~IRenderable() {
        is_ready_= false;
    }

    int Render() {
        int res = DoRender();
        is_ready_ = true;

        return res;
    }

    bool IsReady() const { return is_ready_; }
};

} // namespace eerie_leap
