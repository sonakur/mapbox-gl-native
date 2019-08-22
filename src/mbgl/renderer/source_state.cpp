#include <mbgl/renderer/source_state.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {

using namespace mbgl::style::conversion;

void SourceFeatureState::updateState(const optional<std::string>& sourceLayerID, const std::string& featureID, const Convertible& newState) {

    if (!isObject(newState)) {
        Log::Error(Event::General, "Feature state must be an object");
        return;
    }

    std::string sourceLayer = sourceLayerID ? *sourceLayerID : "";

    std::string stateKey;
    Value stateValue;

    const std::function<optional<Error> (const std::string&, const Convertible&)> convertFn = [&] (const std::string& k, const Convertible& v) -> optional<Error> {
        optional<Value> value = toValue(v);
        if (value) {
            stateValue = *value;
        } else {
            if (isArray(v)) {
                std::vector<Value> array;
                size_t length = arrayLength(v);
                for (size_t i = 0; i < length; i++) {
                    optional<Value> arrayVal = toValue(arrayMember(v, i));
                    if (arrayVal) {
                        array.emplace_back(*arrayVal);
                    }
                }
                std::unordered_map<std::string, Value> result;
                result[k]= array;
                stateValue = result;
                return {};

            } else if (isObject(v)) {
                eachMember(v, convertFn);
            }
        }
        stateKey = k;
        PropertyMap stateResult = {{ stateKey, stateValue }};
        stateChanges[sourceLayer][featureID].insert(stateResult.begin(), stateResult.end());
        return {};
    };

    eachMember(newState, convertFn);
}

PropertyMap SourceFeatureState::getState(const optional<std::string>& sourceLayerID, const std::string& featureId) const {
    std::string sourceLayer = sourceLayerID ? *sourceLayerID : "";
    PropertyMap base, changes;
    auto states = state.find(sourceLayer);
    if (states != state.end()) {
        auto featureStateBase = states->second.find(featureId);
        if (featureStateBase != states->second.end()) {
            base = featureStateBase->second;
        }
    }

    states = stateChanges.find(sourceLayer);
    if (states != stateChanges.end()) {
        auto featureStateChanges = states->second.find(featureId);
        if (featureStateChanges != states->second.end()) {
            changes = featureStateChanges->second;
        }
    }

    PropertyMap& result = changes;
    result.insert(base.begin(), base.end());
    return result;
}

void SourceFeatureState::initializeTileState(Tile &tile) {
    tile.setFeatureState(state);
}

void SourceFeatureState::initializeTileState(RenderTile &tile) {
    tile.setFeatureState(state);
}

void SourceFeatureState::coalesceChanges(std::vector<RenderTile>& tiles) {
    LayerFeatureStates changes;
    for (const auto& layerStatesEntry : stateChanges) {
        const auto& sourceLayer = layerStatesEntry.first;
        FeatureStates layerStates;
        for (const auto& featureStatesEntry : stateChanges[sourceLayer]) {
            const auto& featureID = featureStatesEntry.first;
            state[sourceLayer][featureID].insert(stateChanges[sourceLayer][featureID].begin(),
                                                 stateChanges[sourceLayer][featureID].end());
            layerStates[featureID] = state[sourceLayer][featureID];
        }
        changes[sourceLayer] = layerStates;
    }
    stateChanges.clear();
    if (changes.size() == 0) return;

    for (auto& tile : tiles) {
        tile.setFeatureState(changes);
    }
}

void SourceFeatureState::removeState(const optional<std::string>& sourceLayerID, const optional<std::string>& featureID, const optional<std::string>& stateKey) {
    std::string sourceLayer = sourceLayerID ? *sourceLayerID : "";

    auto layerStates = state.find(sourceLayer);
    if (layerStates != state.end()) {
        if (featureID) {
            auto featureStates = layerStates->second.find(*featureID);
            if (featureStates != layerStates->second.end()) {
                if (stateKey) {
                    auto featureState = featureStates->second.find(*stateKey);
                    if (featureState != featureStates->second.end()) {
                        featureStates->second.erase(featureState);
                    }
                } else {
                    layerStates->second.erase(featureStates);
                }
            }
        } else {
            state.erase(layerStates);
        }
    }

    layerStates = stateChanges.find(sourceLayer);
    if (layerStates != stateChanges.end()) {
        if (featureID) {
            auto featureStates = layerStates->second.find(*featureID);
            if (featureStates != layerStates->second.end()) {
                if (stateKey) {
                    auto featureState = featureStates->second.find(*stateKey);
                    if (featureState != featureStates->second.end()) {
                        featureStates->second.erase(featureState);
                    }
                } else {
                    layerStates->second.erase(featureStates);
                }
            }
        } else {
            stateChanges.erase(layerStates);
        }
    }
}

} // namespace mbgl
