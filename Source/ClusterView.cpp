/*
  ==============================================================================

    ClusterView.cpp
    Created: 2 Mar 2026 11:54:10pm
    Author:  arkwa

  ==============================================================================
*/
//TODO give colors to the groups 
//TODO Add animation when switching cluster modes(maybe?)
//TODO inside the cluster make a menu in top to change the modes and display grouos numbers and what color is that group so give labels
//TODO apply filters for admin and guest can not?
#include "ClusterView.h"
ClusterView::ClusterView(ClusterEngine& e) : engine(e) {}
void ClusterView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    const auto& position = engine.getPositions();

    float width = static_cast<float>(getWidth());
    float height = static_cast<float>(getHeight());

    float r = 4.0f;

    g.setColour(juce::Colours::white);

    for (const auto& p : position)
    {
        float x = p.x * width;
        float y = p.y * height;

        g.fillEllipse(x - r, y - r, r * 2.0f, r * 2.0f);
    } 
}
void ClusterView::resized()
{

}

