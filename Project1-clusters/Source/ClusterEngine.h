
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
// TODO (Sprint 3): Add cluster ID grouping structure using <map>. 

class ClusterEngine
{
public:
    ClusterEngine(SoundLibrary& library);
    void clusterByLength();
	void clusterBySimilarity();
    const std::vector<juce::Point<float>>& getPositions() const;
private:
    SoundLibrary& soundLibrary;
    std::vector<juce::Point<float>> positions;
    float calRMS(Sound *s);
};
