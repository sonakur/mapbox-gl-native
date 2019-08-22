#pragma once

#include <mbgl/style/conversion.hpp>
#include <mbgl/util/feature.hpp>

namespace mbgl {

class Tile;
class RenderTile;
using namespace style::conversion;

class SourceFeatureState {
public:
    SourceFeatureState() = default;
    ~SourceFeatureState() = default;

    void updateState(const optional<std::string>& sourceLayerID, const std::string& featureID, const Convertible& newState);
    PropertyMap getState(const optional<std::string>& sourceLayerID, const std::string& featureID) const;
    void removeState(const optional<std::string>& sourceLayerID, const optional<std::string>& featureID, const optional<std::string>& stateKey);

    void initializeTileState(Tile &tile);
    void initializeTileState(RenderTile &tile);
    void coalesceChanges(std::vector<RenderTile>& tiles);

private:
    LayerFeatureStates state;
    LayerFeatureStates stateChanges;
};

} // namespace mbgl
