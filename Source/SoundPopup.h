/*
  ==============================================================================

    SoundPopup.h
    Created: 20 Apr 2026 11:08:51pm
    Author:  arkwa

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Sound.h"

class SoundPopup : public juce::Component
{
public:
    SoundPopup(Sound* soundShow);
    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    void setPlaying(bool play);
    void setProgress(double newProgress);
    std::function<void(bool)> onPlayToggled;
    std::function<void(double)> onSeekRequested;
private:
    Sound* sound = nullptr;

    juce::Label nameLabel;
    juce::TextButton playButton;
    juce::Slider progressBar;

    juce::Point<int> dragStartMouse;
    juce::Point<int> dragStartComponent;

    juce::ComponentDragger dragger;

    int headerHeight = 32;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundPopup)

};