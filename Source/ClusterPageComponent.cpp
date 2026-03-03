/*
  ==============================================================================

    ClusterPageComponent.cpp
    Created: 3 Mar 2026 5:01:32am
    Author:  arkwa

  ==============================================================================
*/
//temp cluster page please change if there are better ways.
#include "ClusterPageComponent.h"

ClusterPageComponent::ClusterPageComponent(ClusterEngine& en) : eng(en), clusterView(en)
{
    runButton.setButtonText( "Run Clustering" );
    backButton.setButtonText("Back");
    backButton.onClick = [this]()
    {
            if (back)
            {
                back();
            }
    };
    addAndMakeVisible(backButton);

    modeBox.addItem("By Length", 1);
    modeBox.addItem("By Similarity", 2);
    modeBox.setSelectedId(1);
    addAndMakeVisible(modeBox);

    runButton.onClick = [this]()
        {
            if (modeBox.getSelectedId() == 1) {
                eng.clusterByLength();
            }
            else {
                eng.clusterBySimilarity();
            }
            clusterView.repaint();
        }; 
    addAndMakeVisible(runButton);

    addAndMakeVisible(clusterView);
}

void ClusterPageComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a)); //change colors to any IDK
}

void ClusterPageComponent::resized()
{
    auto area = getLocalBounds().reduced(20);

    auto top = area.removeFromTop(50);

    backButton.setBounds(top.removeFromLeft(100));
    modeBox.setBounds(top.removeFromLeft(150).reduced(10));
    runButton.setBounds(top.removeFromLeft(180).reduced(10));

    clusterView.setBounds(area);
}