/*
  ==============================================================================

    ClusterPageComponent.cpp
    Created: 3 Mar 2026 5:01:32am
    Author:  arkwa

  ==============================================================================
*/
//temp cluster page please change if there are better ways.
#include "ClusterPageComponent.h"
#include "UIController.h"

ClusterPageComponent::ClusterPageComponent(ClusterEngine& en, SoundLibrary& library)
    : eng(en), clusterView(en, library), libraryRef(library)
{
    deviceManager.initialise(0, 2, nullptr, true);
    audioSourcePlayer.setSource(&transportSource);
    deviceManager.addAudioCallback(&audioSourcePlayer);
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
    modeBox.onChange = [this]()
        {
            runCurrentMode();
        };

    modeBox.setSelectedId(1);

    addAndMakeVisible(modeBox);

    addAndMakeVisible(clusterView);

    clusterView.onDotClicked = [this, &library](int soundIndex)
        {
            Sound* clickedSound = libraryRef.getSound(soundIndex);
            if (clickedSound == nullptr)
            {
                return;
            }
            stopPreviewPlayback();
            if (soundPopup != nullptr)
            {
                removeChildComponent(soundPopup.get());
                soundPopup.reset();
            }
            currentPopupSound = clickedSound;
            soundPopup = std::make_unique<SoundPopup>(clickedSound);

            soundPopup->onPlayToggled = [this](bool shouldPlay)
                {
                    if (currentPopupSound == nullptr) {
                        return;
                    }
                    if (shouldPlay) {
                        startPreviewPlayback(currentPopupSound);
                    }
                    else {
                        pausePreviewPlayback();
                    }
                };
            soundPopup->onSeekRequested = [this](double normalizedPosition)
                {
                    seekPreviewPlayback(normalizedPosition);
                };
            addAndMakeVisible(soundPopup.get());
            const int popUpWidth = 260;
            const int popUpHeight = 110;
            int popUpX = 60;
            int popUpY = 60;
            const auto& positions = eng.getPositions();

            if (soundIndex >= 0 && soundIndex < static_cast<int>(positions.size()))
            {
                auto clusterBounds = clusterView.getBounds();

                float dotX = clusterBounds.getX() + positions[soundIndex].x * clusterBounds.getWidth();
                float dotY = clusterBounds.getY() + positions[soundIndex].y * clusterBounds.getHeight();
                popUpX = static_cast<int>(dotX) + 12;
                popUpY = static_cast<int>(dotY) - popUpHeight - 12;

                if (popUpY < 0)
                {
                    popUpY = static_cast<int>(dotY) + 12;

                }
                popUpX = juce::jlimit(0, juce::jmax(0, getWidth() - popUpWidth), popUpX);
                popUpY = juce::jlimit(0, juce::jmax(0, getHeight() - popUpHeight), popUpY);
            }
            soundPopup->setBounds(popUpX, popUpY, popUpWidth, popUpHeight);
            soundPopup->toFront(true);

        };

    runCurrentMode();
}
void ClusterPageComponent::seekPreviewPlayback(double normalizedPosition)
{
    if (currentPreviewSeconds <= 0.0) {
        return;
    }

    normalizedPosition = juce::jlimit(0.0, 1.0, normalizedPosition);

    double seekSeconds = normalizedPosition * currentPreviewSeconds;
    transportSource.setPosition(seekSeconds);

    if (soundPopup != nullptr) {
        soundPopup->setProgress(normalizedPosition);
    }
}
void ClusterPageComponent::runCurrentMode()
{
    if (modeBox.getSelectedId() == 1)
    {
        eng.clusterByLength();
    }
    else {
        eng.clusterBySimilarity();
    }
    clusterView.repaint();
    repaint();
}
void ClusterPageComponent::visibilityChanged()
{
    if (isVisible())
    {
        modeBox.setSelectedId(1, juce::dontSendNotification);
        runCurrentMode();
    }
    else {
        stopPreviewPlayback();
        if (soundPopup != nullptr)
        {
            removeChildComponent(soundPopup.get());
            soundPopup.reset();
        }
        currentPopupSound = nullptr;
    }
}

void ClusterPageComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(UIController::clusterPageBg));
}

void ClusterPageComponent::resized()
{
    auto area = getLocalBounds().reduced(20);

    auto top = area.removeFromTop(50);

    backButton.setBounds(top.removeFromLeft(100));
    modeBox.setBounds(top.removeFromLeft(150).reduced(10));


    clusterView.setBounds(area);
    if (soundPopup != nullptr)
    {
        auto bounds = soundPopup->getBounds();
        bounds.setPosition(
            juce::jlimit(0, juce::jmax(0, getWidth() - bounds.getWidth()), bounds.getX()),
            juce::jlimit(0, juce::jmax(0, getHeight() - bounds.getHeight()), bounds.getY())
        );
        soundPopup->setBounds(bounds);
    }
}
ClusterPageComponent::~ClusterPageComponent()
{
    stopTimer();
    transportSource.stop();
    transportSource.setSource(nullptr);
    audioSourcePlayer.setSource(nullptr);
    deviceManager.removeAudioCallback(&audioSourcePlayer);
}
void ClusterPageComponent::startPreviewPlayback(Sound* sound)
{
    if (sound == nullptr || !sound->isValid()) {
        return;
    }
    if (!previewLoaded || sound != currentPopupSound)
    {
        auto& buffer = sound->getAudioBuffer();
        double sampleRate = sound->getSampleRate();

        if (sampleRate <= 0.0 || buffer.getNumSamples() <= 0) {
            return;
        }
        double totalSeconds = (double)buffer.getNumSamples() / sampleRate;

        if (totalSeconds < 20.0) {
            currentPreviewSeconds = totalSeconds * 0.4;
        }
        else {
            currentPreviewSeconds = 10.0;
        }
        int previewSamples = (int)std::round(currentPreviewSeconds * sampleRate);
        previewSamples = juce::jlimit(1, buffer.getNumSamples(), previewSamples);

        transportSource.stop();
        transportSource.setSource(nullptr);
        previewSource.setSound(sound);
        previewSource.setPreviewLengthSamples(previewSamples);
        transportSource.setSource(&previewSource, 0, nullptr, sampleRate);
        transportSource.setPosition(0.0);
        previewLoaded = true;
    }
    transportSource.start();
    if (soundPopup != nullptr) {
        soundPopup->setPlaying(true);
    }
    startTimer(30);
}
void ClusterPageComponent::stopPreviewPlayback()
{
    stopTimer();
    transportSource.stop();
    transportSource.setPosition(0.0);
    previewLoaded = false;
    if (soundPopup != nullptr)
    {
        soundPopup->setPlaying(false);
        soundPopup->setProgress(0.0);
    }
}
void ClusterPageComponent::timerCallback()
{
    if (currentPreviewSeconds <= 0.0) {
        return;
    }
    double currentPos = transportSource.getCurrentPosition();

    if (currentPos >= currentPreviewSeconds || !transportSource.isPlaying())
    {
        stopPreviewPlayback();
        return;
    }

    if (soundPopup != nullptr) {
        soundPopup->setProgress(currentPos / currentPreviewSeconds);
    }
}
void ClusterPageComponent::pausePreviewPlayback()
{
    stopTimer();
    transportSource.stop();

    if (soundPopup != nullptr) {
        soundPopup->setPlaying(false);
    }
}