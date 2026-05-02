#pragma once

#include <JuceHeader.h>
#include "AudioLiveScrollingDisplay.h"
#include "DemoUtilities.h"
#include "UIController.h"



//==============================================================================
/** Audio recorder that writes incoming audio data to a WAV file. */
class AudioRecorder final : public juce::AudioIODeviceCallback
{
public:
    AudioRecorder(juce::AudioThumbnail& thumbnailToUpdate)
        : thumbnail(thumbnailToUpdate)
    {
        backgroundThread.startThread();
    }

    ~AudioRecorder() override
    {
        stop();
    }

    void startRecording(const juce::File& file)
    {
        stop();

        if (sampleRate <= 0)
            return;

        file.deleteFile();
		// If the file stream opens successfully, create a WAV writer and a threaded writer to write to the file in the background
        if (std::unique_ptr<juce::OutputStream> fileStream{ file.createOutputStream() })
        {
            juce::WavAudioFormat wavFormat;
            if (auto writer = wavFormat.createWriterFor(fileStream, juce::AudioFormatWriterOptions{}
                .withSampleRate(sampleRate).withNumChannels(1).withBitsPerSample(16)))
            {
                
                auto* writerPtr = writer.get();
                threadedWriter.reset(new juce::AudioFormatWriter::ThreadedWriter(writer.release(), backgroundThread, 32768));
                thumbnail.reset(writerPtr->getNumChannels(), writerPtr->getSampleRate());
                nextSampleNum = 0;
                {
                    const juce::ScopedLock sl(writerLock);
                    activeWriter = threadedWriter.get();
                }
            }
        }
    }

    // Nullifies everything in ActiveWriter, stops recording
    void stop()
    {
        {
            const juce::ScopedLock sl(writerLock);
            activeWriter = nullptr;
        }
        threadedWriter.reset();
    }

    bool isRecording() const
    {
        return activeWriter.load() != nullptr;
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override
    {
        sampleRate = device->getCurrentSampleRate();
    }

    void audioDeviceStopped() override
    {
        sampleRate = 0;
    }

	// This updates the live waveform thumbnail with incoming audio data and writes to file
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
        float* const* outputChannelData, int numOutputChannels,
        int numSamples, const juce::AudioIODeviceCallbackContext& context) override
    {
        juce::ignoreUnused(context, outputChannelData, numOutputChannels);

        const juce::ScopedLock sl(writerLock);

		// If we're currently recording and the incoming audio has enough channels, write the data to the file and update the thumbnail
        if (activeWriter.load() != nullptr && numInputChannels >= thumbnail.getNumChannels())
        {
            activeWriter.load()->write(inputChannelData, numSamples);
            juce::AudioBuffer<float> buffer(const_cast<float**>(inputChannelData), thumbnail.getNumChannels(), numSamples);
            thumbnail.addBlock(nextSampleNum, buffer, 0, numSamples);
            nextSampleNum += numSamples;
        }
    }

private:
    juce::AudioThumbnail& thumbnail;
    juce::TimeSliceThread backgroundThread{ "Audio Recorder Thread" };
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
    double sampleRate = 0.0;
    juce::int64 nextSampleNum = 0;

    juce::CriticalSection writerLock;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter{ nullptr };
};

//==============================================================================
class RecordingThumbnail final : public juce::Component,
    private juce::ChangeListener
{
public:
    RecordingThumbnail()
    {
        formatManager.registerBasicFormats();
        thumbnail.addChangeListener(this);
    }

    ~RecordingThumbnail() override
    {
        thumbnail.removeChangeListener(this);
    }

    juce::AudioThumbnail& getAudioThumbnail() { return thumbnail; }

    void setDisplayFullThumbnail(bool displayFull)
    {
        displayFullThumb = displayFull;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(UIController::bgRaised));
        g.setColour(juce::Colour(UIController::border));
        g.drawRect(getLocalBounds(), 1);

        if (thumbnail.getTotalLength() > 0.0)
        {
            auto endTime = displayFullThumb ? thumbnail.getTotalLength()
                : juce::jmax(30.0, thumbnail.getTotalLength());

            g.setColour(juce::Colour(UIController::accent));
            thumbnail.drawChannels(g, getLocalBounds().reduced(2), 0.0, endTime, 1.0f);
        }
        else
        {
            g.setColour(juce::Colour(UIController::textDim));
            g.setFont(14.0f);
            g.drawFittedText("(No file recorded)", getLocalBounds(), juce::Justification::centred, 2);
        }
    }

private:
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache{ 10 };
    juce::AudioThumbnail thumbnail{ 512, formatManager, thumbnailCache };

    bool displayFullThumb = false;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        if (source == &thumbnail)
            repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordingThumbnail)
};

//==============================================================================
class PlaybackThumbnail final : public juce::Component,
    private juce::ChangeListener
{
public:
    PlaybackThumbnail()
    {
        formatManager.registerBasicFormats();
        thumbnail.addChangeListener(this);
    }

    ~PlaybackThumbnail() override
    {
        thumbnail.removeChangeListener(this);
    }

    juce::AudioThumbnail& getAudioThumbnail() { return thumbnail; }

    // Update the playback position needle (called from a timer in the parent component)
    void setPlaybackPosition(double currentPosition)
    {
        playbackPosition = currentPosition;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        juce::Rectangle<int> thumbnailBounds = getLocalBounds();

        if (thumbnail.getNumChannels() == 0)
            paintIfNoFileLoaded(g, thumbnailBounds);
        else
            paintIfFileLoaded(g, thumbnailBounds);
    }

private:
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache{ 10 };
    juce::AudioThumbnail thumbnail{ 512, formatManager, thumbnailCache };
    double playbackPosition = 0.0;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        if (source == &thumbnail)
            repaint();
    }

    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
    {
        g.setColour(juce::Colour(UIController::bgRaised));
        g.fillRect(thumbnailBounds);
        g.setColour(juce::Colour(UIController::border));
        g.drawRect(thumbnailBounds, 1);
        g.setColour(juce::Colour(UIController::textDim));
        g.drawFittedText("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
    }

    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
    {
        g.setColour(juce::Colour(UIController::bgRaised));
        g.fillRect(thumbnailBounds);
        g.setColour(juce::Colour(UIController::border));
        g.drawRect(thumbnailBounds, 1);

        g.setColour(juce::Colour(UIController::accent));
        thumbnail.drawChannels(g, thumbnailBounds.reduced(2), 0.0, thumbnail.getTotalLength(), 1.0f);

        // Draw playback position needle
        if (thumbnail.getTotalLength() > 0.0)
        {
            auto proportionPlayed = playbackPosition / thumbnail.getTotalLength();
            auto needleX = thumbnailBounds.getX() + (int)(thumbnailBounds.getWidth() * proportionPlayed);

            g.setColour(juce::Colour(UIController::accentAmber));
            g.drawLine((float)needleX, (float)thumbnailBounds.getY(),
                       (float)needleX, (float)thumbnailBounds.getBottom(), 2.0f);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaybackThumbnail)
};

//==============================================================================
class PitchShiftAudioSource : public juce::PositionableAudioSource
{
public:
    PitchShiftAudioSource(juce::PositionableAudioSource& source)
        : inputSource(&source)
    {
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        currentSampleRate = sampleRate;
        tempBuffer.setSize(2, samplesPerBlockExpected * 2);
        if (inputSource != nullptr)
            inputSource->prepareToPlay(samplesPerBlockExpected * 2, sampleRate);
    }

    void releaseResources() override
    {
        if (inputSource != nullptr)
            inputSource->releaseResources();
        tempBuffer.setSize(0, 0);
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        if (inputSource == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        // If pitch slider not changed, just pass through audio
        if (pitchFactor == 1.0)
        {
            inputSource->getNextAudioBlock(bufferToFill);
            return;
        }

		// Calculate how many samples we need to read from the input source based on the pitch factor
        int samplesNeeded = (int)std::ceil(bufferToFill.numSamples / pitchFactor) + 4;
        if (samplesNeeded < 1)
            samplesNeeded = bufferToFill.numSamples;

        tempBuffer.setSize(bufferToFill.buffer->getNumChannels(), samplesNeeded, false, false, true);
        tempBuffer.clear();

        juce::AudioSourceChannelInfo inputInfo(&tempBuffer, 0, samplesNeeded);
        inputSource->getNextAudioBlock(inputInfo);

        double readPosition = 0.0;
        int numChannels = juce::jmin(bufferToFill.buffer->getNumChannels(), (int)tempBuffer.getNumChannels());

        for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            for (int channel = 0; channel < numChannels; ++channel)
            {
                auto* readPtr = tempBuffer.getReadPointer(channel);
                int readIdx = (int)readPosition;
				// If the read index is within the range of the input buffer, perform linear interpolation to get the output sample value
                if (readIdx >= 0 && readIdx < samplesNeeded - 1)
                {
                    float frac = (float)(readPosition - readIdx);
                    float sample0 = readPtr[readIdx];
                    float sample1 = readPtr[readIdx + 1];
                    float interpolatedValue = sample0 + frac * (sample1 - sample0);
                    bufferToFill.buffer->setSample(channel, bufferToFill.startSample + sample, interpolatedValue);
                }
				// If the read index is out of range, write silence to the output buffer
                else
                {
                    bufferToFill.buffer->setSample(channel, bufferToFill.startSample + sample, 0.0f);
                }
            }
            readPosition += pitchFactor;
        }
		// Clear any remaining channels in the output buffer if the input has fewer channels
        for (int i = numChannels; i < bufferToFill.buffer->getNumChannels(); ++i)
            bufferToFill.buffer->clear(i, bufferToFill.startSample, bufferToFill.numSamples);
    }

    // PositionableAudioSource overrides
    void setNextReadPosition(juce::int64 newPosition) override
    {
        if (inputSource != nullptr)
            inputSource->setNextReadPosition(newPosition);
    }

    juce::int64 getNextReadPosition() const override
    {
        return inputSource != nullptr ? inputSource->getNextReadPosition() : 0;
    }

    juce::int64 getTotalLength() const override
    {
        return inputSource != nullptr ? inputSource->getTotalLength() : 0;
    }

    bool isLooping() const override
    {
        return inputSource != nullptr && inputSource->isLooping();
    }

    void setLooping(bool shouldLoop) override
    {
        if (inputSource != nullptr)
            inputSource->setLooping(shouldLoop);
    }

    void setPitchFactor(double factor)
    {
        pitchFactor = juce::jlimit(0.25, 2.0, factor);
    }

    double getPitchFactor() const { return pitchFactor; }

private:
    juce::PositionableAudioSource* inputSource = nullptr;
    juce::AudioBuffer<float> tempBuffer;
    double pitchFactor = 1.0;
    double currentSampleRate = 44100.0;
};

//==============================================================================
class ReverbAudioSource : public juce::PositionableAudioSource
{
public:
    ReverbAudioSource(juce::PositionableAudioSource& source, juce::Reverb& reverbEffect)
        : inputSource(&source), reverb(reverbEffect)
    {
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        juce::ignoreUnused(sampleRate);
        tempBuffer.setSize(2, samplesPerBlockExpected);
        if (inputSource != nullptr)
            inputSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void releaseResources() override
    {
        if (inputSource != nullptr)
            inputSource->releaseResources();
        tempBuffer.setSize(0, 0);
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        if (inputSource == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        tempBuffer.setSize(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples, false, false, true);
        tempBuffer.clear();

        juce::AudioSourceChannelInfo tempInfo(&tempBuffer, 0, bufferToFill.numSamples);
        inputSource->getNextAudioBlock(tempInfo);

		// Apply reverb to the temp buffer. If the buffer has 2 or more channels, process as stereo; if it has 1 channel, process as mono
        if (tempBuffer.getNumChannels() >= 2)
        {
            auto* leftChannel = tempBuffer.getWritePointer(0);
            auto* rightChannel = tempBuffer.getWritePointer(1);
            reverb.processStereo(leftChannel, rightChannel, bufferToFill.numSamples);
        }
        else if (tempBuffer.getNumChannels() == 1)
        {
            auto* monoChannel = tempBuffer.getWritePointer(0);
            reverb.processMono(monoChannel, bufferToFill.numSamples);
        }

		// Copy the processed audio from the temp buffer to the output buffer
        for (int i = 0; i < bufferToFill.buffer->getNumChannels(); ++i)
            bufferToFill.buffer->copyFrom(i, bufferToFill.startSample, tempBuffer, i, 0, bufferToFill.numSamples);

        // Clear any remaining channels in the output buffer if the input has fewer channels
        for (int i = tempBuffer.getNumChannels(); i < bufferToFill.buffer->getNumChannels(); ++i)
            bufferToFill.buffer->clear(i, bufferToFill.startSample, bufferToFill.numSamples);
    }

    // PositionableAudioSource overrides
    void setNextReadPosition(juce::int64 newPosition) override
    {
        if (inputSource != nullptr)
            inputSource->setNextReadPosition(newPosition);
    }

    juce::int64 getNextReadPosition() const override
    {
        return inputSource != nullptr ? inputSource->getNextReadPosition() : 0;
    }

    juce::int64 getTotalLength() const override
    {
        return inputSource != nullptr ? inputSource->getTotalLength() : 0;
    }

    bool isLooping() const override
    {
        return inputSource != nullptr && inputSource->isLooping();
    }

    void setLooping(bool shouldLoop) override
    {
        if (inputSource != nullptr)
            inputSource->setLooping(shouldLoop);
    }

private:
    juce::PositionableAudioSource* inputSource = nullptr;
    juce::Reverb& reverb;
    juce::AudioBuffer<float> tempBuffer;
};

// This class is needed to prevent BufferingAudioSource from running out of data when user adjusts pitch before playback
// BufferingAudioSource can not handle the case where length is 0, so we return a very large length and make it loop infinitely
class SilentPositionableSource : public juce::PositionableAudioSource
{
public:
    SilentPositionableSource()
    {
        silentBuffer.setSize(2, 512);
        silentBuffer.clear();
    }

    void prepareToPlay(int, double) override {}
    void releaseResources() override {}

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();
    }

    void setNextReadPosition(juce::int64 newPosition) override
    {
        currentPosition = newPosition;
    }

    juce::int64 getNextReadPosition() const override
    {
        return currentPosition;
    }

    juce::int64 getTotalLength() const override
    {
        // Return a large value instead of 0 to satisfy BufferingAudioSource
        return std::numeric_limits<juce::int64>::max();
    }

    bool isLooping() const override
    {
        return true;  // Make it loop so it never runs out
    }

private:
    juce::AudioBuffer<float> silentBuffer;
    juce::int64 currentPosition = 0;
};

//==============================================================================
/** Audio workstation component with recording and playback.
    Designed to be embedded as a child component (not as the main app window).
    Recordings are saved to the same folder selected via "Load Sounds Folder".
*/
class AudioWorkstationComponent : public juce::Component,
    public juce::ChangeListener,
    public juce::Slider::Listener,
    private juce::Timer
{
public:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping
    };

    // Callback to navigate back to the dashboard
    std::function<void()> onBack;

    // Fired after the user saves a finished recording, with the saved file path
    std::function<void(const juce::File&)> onRecordingSaved;

    AudioWorkstationComponent()
        : playbackState(Stopped),
        recorder(recordingThumbnail.getAudioThumbnail())
    {
        setOpaque(true);

        // Back button
        addAndMakeVisible(backButton);
        backButton.setButtonText("Back");
        backButton.onClick = [this] {
            // Stop any active recording/playback before navigating away
            if (recorder.isRecording())
                stopRecording();
            if (playbackState != Stopped)
                changePlaybackState(Stopping);

            if (onBack)
                onBack();
        };

		readAheadThread.startThread();

        // Recording UI
        addAndMakeVisible(recordingLabel);
        recordingLabel.setText("Recording", juce::dontSendNotification);
        recordingLabel.setFont(juce::FontOptions(18.0f, juce::Font::bold));

        addAndMakeVisible(liveAudioScroller);
        addAndMakeVisible(recordingThumbnail);

        addAndMakeVisible(recordButton);
        recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffff5c5c));
        recordButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
        recordButton.setButtonText("Record");
        recordButton.onClick = [this] { recordButtonClicked(); };

        // Playback UI
        addAndMakeVisible(playbackLabel);
        playbackLabel.setText("Playback", juce::dontSendNotification);
        playbackLabel.setFont(juce::FontOptions(18.0f, juce::Font::bold));

        addAndMakeVisible(openButton);
        openButton.setButtonText("Open...");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible(playButton);
        playButton.setButtonText("Play");
        playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
        playButton.setEnabled(false);
        playButton.onClick = [this] { playButtonClicked(); };

        addAndMakeVisible(stopButton);
        stopButton.setButtonText("Stop");
        stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
        stopButton.setEnabled(false);
        stopButton.onClick = [this] { stopButtonClicked(); };

        addAndMakeVisible(saveFilteredButton);
        saveFilteredButton.setButtonText("Save");
        saveFilteredButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
        saveFilteredButton.setEnabled(false);
		saveFilteredButton.onClick = [this] { saveFilteredAudio(); };

        addAndMakeVisible(resetButton);
		resetButton.setButtonText("Reset");
		resetButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
        resetButton.setEnabled(false);
		resetButton.onClick = [this] { resetButtonClicked(); };

        addAndMakeVisible(playbackThumbnail);

        // Volume slider
        addAndMakeVisible(volumeSlider);
        volumeSlider.setRange(0.0, 1.0, 0.01);
        volumeSlider.setValue(0.5);
        volumeSlider.setTextValueSuffix(" Volume");
        volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        volumeSlider.addListener(this);

        // Settings button (audio device selector)
        addAndMakeVisible(settingsButton);
        settingsButton.setButtonText("Settings");
        settingsButton.onClick = [this] { settingsButtonClicked(); };

        // Setup audio - MUST prepare before device callbacks arrive
        formatManager.registerBasicFormats();
        transportSource.addChangeListener(this);
        transportSource.setGain((float)volumeSlider.getValue());
        playbackThumbnail.getAudioThumbnail().addChangeListener(this);

        deviceManager.addAudioCallback(&liveAudioScroller);
        deviceManager.addAudioCallback(&audioSourcePlayer);
        deviceManager.addAudioCallback(&recorder);

        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [this](bool granted)
            {
                deviceManager.initialise(granted ? 2 : 0, 2, nullptr, true, {}, nullptr);
            });

        setupEffects();

        startTimerHz(30);
    }

    ~AudioWorkstationComponent() override
    {
        stopTimer();
        transportSource.removeChangeListener(this);
        deviceManager.removeAudioCallback(&liveAudioScroller);
        deviceManager.removeAudioCallback(&recorder);
        deviceManager.removeAudioCallback(&audioSourcePlayer);
        transportSource.setSource(nullptr);
        audioSourcePlayer.setSource(nullptr);
    }

    /** Set the folder where recordings should be saved (same as the loaded sounds folder). */
    void setSoundsFolder(const juce::File& folder)
    {
        soundsFolder = folder;
    }

    bool isAudioDeviceInitialized() const
    {
        auto* device = deviceManager.getCurrentAudioDevice();
        return device != nullptr && device->getActiveInputChannels().countNumberOfSetBits() > 0;
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(UIController::bg));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);

        // Back button at top
        auto topBar = area.removeFromTop(36);
        backButton.setBounds(topBar.removeFromLeft(100).reduced(4));
        area.removeFromTop(5);

        // Recording section
        recordingLabel.setBounds(area.removeFromTop(30).reduced(4));
        liveAudioScroller.setBounds(area.removeFromTop(80).reduced(4));
        recordingThumbnail.setBounds(area.removeFromTop(80).reduced(4));
        recordButton.setBounds(area.removeFromTop(36).removeFromLeft(140).reduced(4));

        // Playback section
        area.removeFromTop(10);
        playbackLabel.setBounds(area.removeFromTop(30).reduced(4));

        auto playbackButtonArea = area.removeFromTop(30).reduced(4);
        openButton.setBounds(playbackButtonArea.removeFromLeft(100));
        playbackButtonArea.removeFromLeft(5);
        playButton.setBounds(playbackButtonArea.removeFromLeft(80));
        playbackButtonArea.removeFromLeft(5);
        stopButton.setBounds(playbackButtonArea.removeFromLeft(80));
        playbackButtonArea.removeFromLeft(10);
        saveFilteredButton.setBounds(playbackButtonArea.removeFromLeft(120));
        playbackButtonArea.removeFromLeft(10);
        settingsButton.setBounds(playbackButtonArea.removeFromLeft(100));
        playbackButtonArea.removeFromLeft(10);
        resetButton.setBounds(playbackButtonArea.removeFromLeft(100));


		// Volume slider sits just below the playback buttons
		volumeSlider.setBounds(area.removeFromTop(30).reduced(4));
        area.removeFromTop(5);

		// Pitch slider
		pitchSlider.setBounds(area.removeFromTop(30).reduced(4));
        area.removeFromTop(5);

		// Reverb dials below volume
		auto reverbArea = area.removeFromTop(100).reduced(4);
		reverbWetSlider.setBounds(reverbArea.removeFromLeft(150));
        reverbArea.removeFromLeft(10);
		reverbRoomSizeSlider.setBounds(reverbArea.removeFromLeft(150));
        reverbArea.removeFromLeft(10);
		reverbDampingSlider.setBounds(reverbArea.removeFromLeft(150));

		playbackThumbnail.setBounds(area.reduced(4));
	}

	void sliderValueChanged(juce::Slider* slider) override
	{
		bool toggleReset = true;

        if (slider == &volumeSlider)
        {
            transportSource.setGain((float)volumeSlider.getValue());
            toggleReset = false;  // Don't enable reset button just for volume changes
        }

		if (slider == &pitchSlider)
            if (pitchShiftSource != nullptr)
			    pitchShiftSource->setPitchFactor(pitchSlider.getValue());

		if (slider == &reverbWetSlider)
		{
			reverbParams.wetLevel = static_cast<float>(reverbWetSlider.getValue());
			reverb.setParameters(reverbParams);
		}

		if (slider == &reverbRoomSizeSlider)
		{
			reverbParams.roomSize = static_cast<float>(reverbRoomSizeSlider.getValue());
			reverb.setParameters(reverbParams);
		}

		if (slider == &reverbDampingSlider)
		{
			reverbParams.damping = static_cast<float>(reverbDampingSlider.getValue());
			reverb.setParameters(reverbParams);
		}

        if (toggleReset)
			resetButton.setEnabled(true);
	}

    void timerCallback() override
    {
        // Push the current transport position to the thumbnail so the needle moves
        playbackThumbnail.setPlaybackPosition(transportSource.getCurrentPosition());
    }

    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
                changePlaybackState(Playing);
            else if ((playbackState == Stopping) || (playbackState == Playing))
                changePlaybackState(Stopped);
            else if (playbackState == Pausing)
                changePlaybackState(Paused);
        }

        if (source == &playbackThumbnail.getAudioThumbnail())
            repaint();
    }

private:
    void setupEffects()
    {
        // Setup reverb to wetLevel 0 and dryLevel 1 means no reverb by default, when i opened a recording, it would have
        // that sound effect built in, so fixed that here
        reverbParams.roomSize = 0.0f;
        reverbParams.damping  = 0.0f;
        reverbParams.wetLevel = 0.0f;
        reverbParams.dryLevel = 1.0f;
        reverbParams.width    = 1.0f;
        reverbParams.freezeMode = 0.0f;
        reverb.setParameters(reverbParams);

        // Setup UI for effects
        auto setupReverbSlider = [this](juce::Slider& slider, const char* suffix)
            {
                addAndMakeVisible(slider);
                slider.setRange(0.0, 1.0, 0.01);
                slider.setValue(0.0);
                slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
                slider.setTextValueSuffix(suffix);
                slider.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
                slider.addListener(this);
            };

        setupReverbSlider(reverbWetSlider, " Wet");
        setupReverbSlider(reverbRoomSizeSlider, " Room Size");
        setupReverbSlider(reverbDampingSlider, " Damping");

        // Setup pitch slider
        addAndMakeVisible(pitchSlider);
        pitchSlider.setRange(0.25, 2.0, 0.01);
        pitchSlider.setValue(1.0);
        pitchSlider.setTextValueSuffix(" Pitch");
        pitchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        pitchSlider.addListener(this);
    }

    // Modified openButtonClicked to include effects chain
    void openButtonClicked()
    {
        juce::File startDir = soundsFolder.isDirectory() ? soundsFolder : juce::File{};

        chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...", startDir, "*.wav");

        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) { loadAudioFile(fc.getResult()); });
    }

public:
    // Loads an audio file into the workstation, rebuilding the effects chain.
    // Public so other views (e.g. the sound list's Edit button) can hand a file in directly.
    void loadAudioFile(const juce::File& file)
    {
        // Saves file to a member variable so applied effects can be saved
        currentFile = file;

        // Update the sounds folder to the directory the user selected from
        if (file != juce::File{})
            soundsFolder = file.getParentDirectory();

        if (file != juce::File{})
        {
            auto* reader = formatManager.createReaderFor(file);
            if (reader != nullptr)
            {
                // Reset transport state before tearing down the chain
                transportSource.stop();
                playbackState = Stopped;
                playButton.setButtonText("Play");
                playButton.setEnabled(false);
                stopButton.setEnabled(false);
                stopButton.setButtonText("Stop");
                saveFilteredButton.setEnabled(false);

                // Disconnect transport first — this releases the BufferingAudioSource
                // which holds a raw pointer into the old effects chain
                transportSource.setSource(nullptr);
                audioSourcePlayer.setSource(nullptr);

                // Now safe to destroy the old chain
                reverbSource.reset();
                pitchShiftSource.reset();
                readerSource.reset();

                // Build new chain
                auto fileSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                pitchShiftSource = std::make_unique<PitchShiftAudioSource>(*fileSource);
                reverbSource = std::make_unique<ReverbAudioSource>(*pitchShiftSource, reverb);

                transportSource.setSource(reverbSource.get(), 32768, &readAheadThread, reader->sampleRate);
                audioSourcePlayer.setSource(&transportSource);

                readerSource = std::move(fileSource);

                playButton.setEnabled(true);
                saveFilteredButton.setEnabled(true);
                playbackThumbnail.getAudioThumbnail().setSource(new juce::FileInputSource(file));
                openButton.setButtonText(file.getFileName());

                reverb.setSampleRate(reader->sampleRate);
                reverb.setParameters(reverbParams);
            }
        }
    }

private:
    void recordButtonClicked()
    {
        if (recorder.isRecording())
            stopRecording();
        else
            startRecording();
    }

    void startRecording()
    {
        if (!juce::RuntimePermissions::isGranted(juce::RuntimePermissions::writeExternalStorage))
        {
            juce::Component::SafePointer<AudioWorkstationComponent> safeThis(this);
            juce::RuntimePermissions::request(juce::RuntimePermissions::writeExternalStorage,
                [safeThis](bool granted) mutable { if (granted) safeThis->startRecording(); });
            return;
        }

        juce::File parentDir = soundsFolder.isDirectory()
            ? soundsFolder
            : juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);

        lastRecording = parentDir.getNonexistentChildFile("Recording", ".wav");

        recorder.startRecording(lastRecording);
        recordButton.setButtonText("Stop");
        recordingThumbnail.setDisplayFullThumbnail(false);
    }

    void stopRecording()
    {
        recorder.stop();

        juce::File suggestedFile = soundsFolder.isDirectory()
            ? soundsFolder.getNonexistentChildFile("Recording", ".wav")
            : lastRecording;

        chooser = std::make_unique<juce::FileChooser>("Save recorded file as...",
            suggestedFile.exists() ? suggestedFile : lastRecording, "*.wav");

        chooser->launchAsync(juce::FileBrowserComponent::saveMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::warnAboutOverwriting,
            [this](const juce::FileChooser& c)
            {
                auto destFile = c.getResult();
                if (destFile != juce::File{})
                {
                    // Update the sounds folder to the directory the user selected
                    soundsFolder = destFile.getParentDirectory();
                    lastRecording.copyFileTo(destFile);
                    if (onRecordingSaved) onRecordingSaved(destFile);
                }

                recordButton.setButtonText("Record");
                recordingThumbnail.setDisplayFullThumbnail(true);
            });
    }

    void settingsButtonClicked()
    {
        // Stop playback before opening settings to avoid audio device conflicts
        if (playbackState == Playing || playbackState == Pausing || playbackState == Paused)
            stopButtonClicked();

        auto deviceSelector = std::make_unique<juce::AudioDeviceSelectorComponent>(
            deviceManager, 0, 256, 0, 256, false, false, false, false);
        deviceSelector->setSize(500, 500);
        juce::CallOutBox::launchAsynchronously(std::move(deviceSelector), settingsButton.getScreenBounds(), nullptr);
    }

    void playButtonClicked()
    {
        if ((playbackState == Stopped) || (playbackState == Paused))
            changePlaybackState(Starting);
        else if (playbackState == Playing)
            changePlaybackState(Pausing);
    }

    void stopButtonClicked()
    {
        if (playbackState == Paused)
            changePlaybackState(Stopped);
        else
            changePlaybackState(Stopping);
    }

    void saveFilteredAudio()
    {
        if (currentFile == juce::File{} || pitchShiftSource == nullptr)
            return;

        juce::File suggested = soundsFolder.isDirectory()
            ? soundsFolder.getNonexistentChildFile("Processed", ".wav")
            : juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Processed.wav");

        chooser = std::make_unique<juce::FileChooser>("Save processed audio as...", suggested, "*.wav");

        chooser->launchAsync(
            juce::FileBrowserComponent::saveMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::warnAboutOverwriting,
            [this](const juce::FileChooser& fc)
            {
                auto destFile = fc.getResult();
                if (destFile == juce::File{})
                    return;

                // Update the sounds folder to the directory the user selected
                soundsFolder = destFile.getParentDirectory();

                auto* reader = formatManager.createReaderFor(currentFile);
                if (reader == nullptr)
                    return;

                const int    blockSize = 4096;
                const double sr = reader->sampleRate;
                const int    numCh = juce::jmin((int)reader->numChannels, 2);
                const double pitch = pitchShiftSource->getPitchFactor();

                // Total output length changes with pitch: faster = shorter, slower = longer
                const juce::int64 totalOut = (juce::int64)(reader->lengthInSamples / pitch);

                // Build a fresh offline effects chain — avoids touching the live chain mid-playback
                auto offlineReader = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                PitchShiftAudioSource offlinePitch(*offlineReader);
                offlinePitch.setPitchFactor(pitch);

                juce::Reverb offlineReverb;           // fresh state, no bleed from live playback
                offlineReverb.setSampleRate(sr);
                offlineReverb.setParameters(reverbParams);

                ReverbAudioSource offlineReverbSrc(offlinePitch, offlineReverb);

                offlineReader->prepareToPlay(blockSize, sr);
                offlinePitch.prepareToPlay(blockSize, sr);
                offlineReverbSrc.prepareToPlay(blockSize, sr);

                destFile.deleteFile();

                if (auto fileStream = destFile.createOutputStream())
                {
                    juce::WavAudioFormat wavFormat;
                    if (auto* writer = wavFormat.createWriterFor(
                        fileStream.release(),   // transfer ownership to the writer
                        sr,
                        (unsigned int)numCh,
                        16,
                        {},
                        0))
                    {
                        std::unique_ptr<juce::AudioFormatWriter> writerOwner(writer);

                        juce::int64 totalInputSamples = reader->lengthInSamples;
                        juce::AudioBuffer<float> buffer(numCh, blockSize);

                        while (offlineReader->getNextReadPosition() < totalInputSamples)
                        {
                            buffer.clear();
                            juce::AudioSourceChannelInfo info(&buffer, 0, blockSize);
                            offlineReverbSrc.getNextAudioBlock(info);
                            writerOwner->writeFromAudioSampleBuffer(buffer, 0, blockSize);
                        }
                    }
                }

                offlineReverbSrc.releaseResources();
            });
    }

    void resetButtonClicked()
    {
        // Reset all effects to default values
        pitchSlider.setValue(1.0);
        reverbWetSlider.setValue(0.0);
        reverbRoomSizeSlider.setValue(0.0);
        reverbDampingSlider.setValue(0.0);
        if (pitchShiftSource != nullptr)
			pitchShiftSource->setPitchFactor(1.0);
	}

    void changePlaybackState(TransportState newState)
    {
        if (playbackState != newState)
        {
            playbackState = newState;

            switch (playbackState)
            {
            case Stopped:
                playButton.setButtonText("Play");
                stopButton.setButtonText("Stop");
                stopButton.setEnabled(false);
                transportSource.setPosition(0.0);
                break;

            case Starting:
                transportSource.start();
                break;

            case Playing:
                playButton.setButtonText("Pause");
                stopButton.setButtonText("Stop");
                stopButton.setEnabled(true);
                saveFilteredButton.setEnabled(true);
                break;

            case Pausing:
                transportSource.stop();
                break;

            case Paused:
                playButton.setButtonText("Resume");
                stopButton.setButtonText("Restart");
                saveFilteredButton.setEnabled(true);
                break;

            case Stopping:
                transportSource.stop();
                break;
            }
        }
    }

    // === UI Components ===
    juce::TextButton backButton;
    juce::Label recordingLabel;
    LiveScrollingAudioDisplay liveAudioScroller;
    RecordingThumbnail recordingThumbnail;
    juce::TextButton recordButton;
    juce::File lastRecording;
    AudioRecorder recorder;

    juce::Label playbackLabel;
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton saveFilteredButton;
    juce::TextButton settingsButton;
    juce::TextButton resetButton;
    juce::Slider volumeSlider;
    PlaybackThumbnail playbackThumbnail;

    // === Effects UI ===
    juce::Slider reverbWetSlider;
    juce::Slider reverbRoomSizeSlider;
    juce::Slider reverbDampingSlider;
    juce::Slider pitchSlider;

    // === Audio Engine ===
    juce::AudioSourcePlayer audioSourcePlayer;
    juce::AudioDeviceManager deviceManager;
    juce::AudioFormatManager formatManager;
    juce::File soundsFolder;
    juce::File currentFile;
    bool isDeviceInitialized = false;

    // === Transport & Sources ===
    juce::TimeSliceThread readAheadThread{ "Read Ahead Thread" };
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState playbackState;

    // === Effects Processors ===
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;
    std::unique_ptr<PitchShiftAudioSource> pitchShiftSource;
    std::unique_ptr<ReverbAudioSource> reverbSource;

    // === Thread Safety ===
    juce::CriticalSection audioSourceLock;

    // === Misc ===
    std::unique_ptr<juce::FileChooser> chooser;
	bool effectsEnabled = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioWorkstationComponent)
};
