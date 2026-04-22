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
#include "SoundPopup.h"
#include <memory>

class ClusterPageComponent : public juce::Component, private juce::Timer
{
public:
    ClusterPageComponent(ClusterEngine& en, SoundLibrary& library);
    ~ClusterPageComponent() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    void runCurrentMode();
    std::function<void()> back;
    void visibilityChanged() override;



private:
    ClusterEngine& eng;
    ClusterView clusterView;
    bool previewLoaded = false;
    juce::TextButton backButton;
    juce::ComboBox modeBox;
    juce::TextButton runButton;
    std::unique_ptr<SoundPopup> soundPopup;
    class BufferPreviewAudioSource : public juce::PositionableAudioSource
    {
    public:
        BufferPreviewAudioSource() = default;
        void setSound(Sound* newSound)
        {
            sound = newSound;
            currentSample = 0;
        }
        void setPreviewLengthSamples(int newLength)
        {
            previewLengthSamples = juce::jmax(0, newLength);
            currentSample = 0;
        }
        void prepareToPlay(int, double) override {}
        void releaseResources() override {}
        void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
        {
            bufferToFill.clearActiveBufferRegion();

            if (sound == nullptr || !sound->isValid())
            {
                return;
            }
            auto& sourceBuffer = sound->getAudioBuffer();
            int numChannels = sourceBuffer.getNumChannels();
            int totalSamples = juce::jmin(sourceBuffer.getNumSamples(), previewLengthSamples);

            if (currentSample >= totalSamples) {
                return;
            }
            int samplesToCopy = juce::jmin(bufferToFill.numSamples, totalSamples - currentSample);

            for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
            {
                int sourceChannel = juce::jmin(ch, numChannels - 1);

                bufferToFill.buffer->copyFrom(ch, bufferToFill.startSample, sourceBuffer, sourceChannel, currentSample, samplesToCopy
                );
            }

            currentSample += samplesToCopy;
        }

        void setNextReadPosition(juce::int64 newPosition) override
        {
            currentSample = (int)juce::jlimit<juce::int64>(0, previewLengthSamples, newPosition);
        }
        juce::int64 getNextReadPosition() const override
        {
            return currentSample;
        }
        juce::int64 getTotalLength() const override
        {
            return previewLengthSamples;
        }
        bool isLooping() const override
        {
            return false;
        }
    private:
        Sound* sound = nullptr;
        int currentSample = 0;
        int previewLengthSamples = 0;
    };
    SoundLibrary& libraryRef;
    Sound* currentPopupSound = nullptr;

    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer audioSourcePlayer;
    juce::AudioTransportSource transportSource;
    BufferPreviewAudioSource previewSource;

    double currentPreviewSeconds = 0.0;

    void timerCallback() override;
    void stopPreviewPlayback();
    void startPreviewPlayback(Sound* sound);
    void seekPreviewPlayback(double normalizedPosition);
    void pausePreviewPlayback();
};