
/*
  ==============================================================================

    ClusterEngine.h
    Created: 3 Mar 2026 12:14:32am
    Author:  arkwa

  ==============================================================================
*/

#pragma once
#include <vector>
#include <JuceHeader.h>
#include <cmath>
#include <algorithm>
#include "Soundlibrary.h"
#include <limits>


class ClusterEngine
{
public:
    ClusterEngine(SoundLibrary& library);
    void clusterByLength();
    void clusterBySimilarity();
    const std::vector<juce::Point<float>>& getPositions() const;
    const std::vector<int>& getClusterIds() const;
    bool isLengthMode() const;
private:
    SoundLibrary& soundLibrary;
    std::vector<juce::Point<float>> positions;
    //std::vector<float> extractWaveformFeature(Sound* s, int target);
    //float waveformDistance(const std::vector<float>& a, const std::vector<float>& b);
    std::vector<int> clusterIds;
    bool lengthMode = true;

};

