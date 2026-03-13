#pragma once

#include "ofMain.h"

struct SharedSliceState {
    ofTexture texture;
    bool ready = false;
    std::vector<float> sliceEdges;

    void setupSlices(int count) {
        if (count < 1) {
            sliceEdges.clear();
            return;
        }
        sliceEdges.resize(count + 1);
        for (int i = 0; i <= count; ++i) {
            sliceEdges[i] = static_cast<float>(i) / static_cast<float>(count);
        }
    }

    int getSliceCount() const {
        return sliceEdges.size() > 1 ? static_cast<int>(sliceEdges.size() - 1) : 0;
    }

    ofRectangle getSliceArea(int index) const {
        if (!texture.isAllocated() || getSliceCount() == 0) {
            return {};
        }
        index = ofClamp(index, 0, getSliceCount() - 1);
        auto y1 = sliceEdges[index] * texture.getHeight();
        auto y2 = sliceEdges[index + 1] * texture.getHeight();
        return ofRectangle(0, y1, texture.getWidth(), y2 - y1);
    }
};
