/*
  ==============================================================================

    ClusterPageComponent.h
    Created: 3 Mar 2026 5:01:32am
    Author:  arkwa

  ==============================================================================
*/
//temp. cluster page

#pragma once
#include <JuceHeader.h>
#include "ClusterView.h"
#include "ClusterEngine.h"

class ClusterPageComponent : public juce::Component
{
public:
    ClusterPageComponent(ClusterEngine& en);

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void()> back;


private:
    ClusterEngine& eng;
    ClusterView clusterView;

    juce::TextButton backButton;
    juce::ComboBox modeBox;
    juce::TextButton runButton;
};