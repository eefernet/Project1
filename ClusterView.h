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


class ClusterView : public juce::Component
{
    public:
        ClusterView(ClusterEngine& engine);
        void paint(juce::Graphics& g) override;
        void resized() override;
private: 
    ClusterEngine& engine;

};
