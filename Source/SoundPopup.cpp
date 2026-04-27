/*
  ==============================================================================

    SoundPopup.cpp
    Created: 20 Apr 2026 11:08:51pm
    Author:  arkwa

  ==============================================================================
*/

#include "SoundPopup.h"
#include "UIController.h"

SoundPopup::SoundPopup(Sound* soundShow) : sound(soundShow)
{
    nameLabel.setText(sound != nullptr ? sound->getName() : "Unknown Sound", juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    nameLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(nameLabel);

    playButton.setButtonText("play");

    playButton.setClickingTogglesState(true);
    playButton.onClick = [this]()
        {
            bool isPlayingNow = playButton.getToggleState();
            playButton.setButtonText(isPlayingNow ? "Pause" : "Play");

            if (onPlayToggled)
            {
                onPlayToggled(isPlayingNow);
            }
        };

    addAndMakeVisible(playButton);

    progressBar.setSliderStyle(juce::Slider::LinearBar);
    progressBar.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    progressBar.setRange(0.0, 1.0, 0.0);
    progressBar.setValue(0.0);
    progressBar.setEnabled(true);
    progressBar.onDragEnd = [this]()
        {
            if (onSeekRequested) {
                onSeekRequested(progressBar.getValue());
            }
        };
    addAndMakeVisible(progressBar);
    closeButton.setButtonText("x");
    closeButton.onClick = [this]()
        {
            if (onCloseRequest) {
                onCloseRequest();
            }
        };
    addAndMakeVisible(closeButton);

}
void SoundPopup::setPlaying(bool play)
{
    playButton.setToggleState(play, juce::dontSendNotification);
    playButton.setButtonText(play ? "Pause" : "Play");
}
void SoundPopup::setProgress(double newProgress)
{
    progressBar.setValue(juce::jlimit(0.0, 1.0, newProgress), juce::dontSendNotification);
}
void SoundPopup::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(UIController::bgRaised));
    g.fillRoundedRectangle(bounds, 10.0f);
    g.setColour(juce::Colour(UIController::border));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 10.0f, 1.0f);
    g.setColour(juce::Colour(UIController::bgHover));
    g.fillRoundedRectangle(0.0f, 0.0f, (float)getWidth(), (float)headerHeight, 10.0f);
    g.setColour(juce::Colour(UIController::borderHot));
    g.drawLine(0.0f, (float)headerHeight, (float)getWidth(), (float)headerHeight, 1.0f);
}
void SoundPopup::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto top = area.removeFromTop(headerHeight);
    closeButton.setBounds(top.removeFromRight(45));
    top.removeFromRight(6);
    nameLabel.setBounds(top);
    area.removeFromTop(8);
    auto buttonrow = area.removeFromTop(32);
    playButton.setBounds(buttonrow.removeFromLeft(80));
    area.removeFromTop(10);
    progressBar.setBounds(area.removeFromTop(24));


}
void SoundPopup::mouseDown(const juce::MouseEvent& event)
{
    if (getParentComponent() != nullptr)
    {
        dragger.startDraggingComponent(this, event);

    }
}
void SoundPopup::mouseDrag(const juce::MouseEvent& event)
{
    if (getParentComponent() != nullptr)
    {
        dragger.dragComponent(this, event, nullptr);

        auto bounds = getBounds();

        int maxX = getParentComponent()->getWidth() - getWidth();
        int maxY = getParentComponent()->getHeight() - getHeight();

        bounds.setX(juce::jlimit(0, juce::jmax(0, maxX), bounds.getX()));
        bounds.setY(juce::jlimit(0, juce::jmax(0, maxY), bounds.getY()));

        setBounds(bounds);
    }
}