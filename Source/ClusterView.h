/*
  ==============================================================================

    ClusterView.h
    Created: 2 Mar 2026 11:54:10pm
    Author:  arkwa

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "ClusterEngine.h"
#include "Soundlibrary.h"


class ClusterView : public juce::Component
{
public:
    ClusterView(ClusterEngine& engine, SoundLibrary& library);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    std::function<void(int)> onDotClicked;

private:
    int getDotAtPosition(juce::Point<float> mousePos) const;
    ClusterEngine& engine;
    juce::Colour getClusterColour(int clusterId) const;
    juce::String getClusterLabel(int clusterId, int clusterCount) const;
    SoundLibrary& soundLibrary;
    int hoveredDotIndex = -1;
    float dotRadius = 4.0f;

};
