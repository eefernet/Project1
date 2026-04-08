#pragma once

#include <JuceHeader.h>
#include "AudioLiveScrollingDisplay.h"
#include "DemoUtilities.h"

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

        if (sampleRate > 0)
        {
            file.deleteFile();

            if (std::unique_ptr<juce::OutputStream> fileStream{ file.createOutputStream() })
            {
                juce::WavAudioFormat wavFormat;
                using Opts = juce::AudioFormatWriterOptions;

                if (auto writer = wavFormat.createWriterFor(fileStream, Opts{}.withSampleRate(sampleRate)
                    .withNumChannels(1)
                    .withBitsPerSample(16)))
                {
                    auto* writerPtr = writer.get();
                    threadedWriter.reset(new juce::AudioFormatWriter::ThreadedWriter(writer.release(), backgroundThread, 32768));

                    thumbnail.reset(writerPtr->getNumChannels(), writerPtr->getSampleRate());
                    nextSampleNum = 0;

                    const juce::ScopedLock sl(writerLock);
                    activeWriter = threadedWriter.get();
                }
            }
        }
    }

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

    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
        float* const* outputChannelData, int numOutputChannels,
        int numSamples, const juce::AudioIODeviceCallbackContext& context) override
    {
        juce::ignoreUnused(context, outputChannelData, numOutputChannels);

        const juce::ScopedLock sl(writerLock);

        if (activeWriter.load() != nullptr && numInputChannels >= thumbnail.getNumChannels())
        {
            activeWriter.load()->write(inputChannelData, numSamples);

            juce::AudioBuffer<float> buffer(const_cast<float**>(inputChannelData), thumbnail.getNumChannels(), numSamples);
            thumbnail.addBlock(nextSampleNum, buffer, 0, numSamples);
            nextSampleNum += numSamples;
        }
        for (int i = 0; i < numOutputChannels; ++i)
            if (outputChannelData[i] != nullptr)
                juce::FloatVectorOperations::clear(outputChannelData[i], numSamples);
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
        g.fillAll(juce::Colours::darkgrey);
        g.setColour(juce::Colours::lightgrey);

        if (thumbnail.getTotalLength() > 0.0)
        {
            auto endTime = displayFullThumb ? thumbnail.getTotalLength()
                : juce::jmax(30.0, thumbnail.getTotalLength());

            auto thumbArea = getLocalBounds();
            thumbnail.drawChannels(g, thumbArea.reduced(2), 0.0, endTime, 1.0f);
        }
        else
        {
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
        g.setColour(juce::Colours::darkgrey);
        g.fillRect(thumbnailBounds);
        g.setColour(juce::Colours::white);
        g.drawFittedText("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
    }

    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
    {
        g.setColour(juce::Colours::midnightblue);
        g.fillRect(thumbnailBounds);

        juce::ColourGradient gradient(juce::Colours::red, 0.0f, (float)getHeight(),
            juce::Colours::yellow, (float)getWidth(), (float)getHeight(), false);
        g.setGradientFill(gradient);

        thumbnail.drawChannels(g, thumbnailBounds, 0.0, thumbnail.getTotalLength(), 1.0f);

        // Draw playback position needle
        if (thumbnail.getTotalLength() > 0.0)
        {
            auto proportionPlayed = playbackPosition / thumbnail.getTotalLength();
            auto needleX = thumbnailBounds.getX() + (int)(thumbnailBounds.getWidth() * proportionPlayed);

            g.setColour(juce::Colours::white);
            g.drawLine((float)needleX, (float)thumbnailBounds.getY(),
                       (float)needleX, (float)thumbnailBounds.getBottom(), 2.0f);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaybackThumbnail)
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

        // Setup audio
        formatManager.registerBasicFormats();
        transportSource.addChangeListener(this);
        transportSource.setGain((float)volumeSlider.getValue());
        playbackThumbnail.getAudioThumbnail().addChangeListener(this);

        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [this](bool granted)
            {
                int numInputChannels = granted ? 2 : 0;
                deviceManager.initialise(numInputChannels, 2, nullptr, true, {}, nullptr);
            });

        deviceManager.addAudioCallback(&liveAudioScroller);
        deviceManager.addAudioCallback(&recorder);

        // Drive the playback position needle at ~30Hz
        startTimerHz(30);
    }

    ~AudioWorkstationComponent() override
    {
        stopTimer();
        transportSource.removeChangeListener(this);
        deviceManager.removeAudioCallback(&recorder);
        deviceManager.removeAudioCallback(&liveAudioScroller);
        transportSource.setSource(nullptr);
    }

    /** Set the folder where recordings should be saved (same as the loaded sounds folder). */
    void setSoundsFolder(const juce::File& folder)
    {
        soundsFolder = folder;
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(getUIColourIfAvailable(juce::LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
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
        settingsButton.setBounds(playbackButtonArea.removeFromLeft(100));

        // Volume slider sits just below the playback buttons
        volumeSlider.setBounds(area.removeFromTop(30).reduced(4));

        playbackThumbnail.setBounds(area.reduced(4));
    }

    void sliderValueChanged(juce::Slider* slider) override
    {
        if (slider == &volumeSlider)
            transportSource.setGain((float)volumeSlider.getValue());
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
                [safeThis](bool granted) mutable
                {
                    if (granted)
                        safeThis->startRecording();
                });
            return;
        }

        // If a sounds folder has been set, save directly there. Otherwise use Documents.
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

        // Default save location is the sounds folder if available
        juce::File suggestedFile = soundsFolder.isDirectory()
            ? soundsFolder.getNonexistentChildFile("Recording", ".wav")
            : lastRecording;

        chooser = std::make_unique<juce::FileChooser>("Save recorded file as...",
            suggestedFile.exists() ? suggestedFile : lastRecording,
            "*.wav");

        auto chooserFlags = juce::FileBrowserComponent::saveMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::warnAboutOverwriting;

        chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& c)
            {
                auto destFile = c.getResult();
                if (destFile != juce::File{})
                {
                    if (juce::FileInputStream inputStream(lastRecording); inputStream.openedOk())
                    {
                        if (auto outputStream = makeOutputStream(juce::URL(destFile)))
                            outputStream->writeFromInputStream(inputStream, -1);
                    }
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

    void openButtonClicked()
    {
        // Default to sounds folder if set
        juce::File startDir = soundsFolder.isDirectory()
            ? soundsFolder
            : juce::File{};

        chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
            startDir,
            "*.wav");
        auto chooserFlags = juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();

                if (file != juce::File{})
                {
                    auto* reader = formatManager.createReaderFor(file);

                    if (reader != nullptr)
                    {
                        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
                        playButton.setEnabled(true);
                        readerSource.reset(newSource.release());
                        playbackThumbnail.getAudioThumbnail().setSource(new juce::FileInputSource(file));
                        openButton.setButtonText(file.getFileName());
                    }
                }
            });
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
                break;

            case Pausing:
                transportSource.stop();
                break;

            case Paused:
                playButton.setButtonText("Resume");
                stopButton.setButtonText("Restart");
                break;

            case Stopping:
                transportSource.stop();
                break;
            }
        }
    }

    juce::AudioDeviceManager deviceManager;
    juce::File soundsFolder;

    // Back button
    juce::TextButton backButton;

    // Recording components
    juce::Label recordingLabel;
    LiveScrollingAudioDisplay liveAudioScroller;
    RecordingThumbnail recordingThumbnail;
    AudioRecorder recorder;
    juce::TextButton recordButton;
    juce::File lastRecording;

    // Playback components
    juce::Label playbackLabel;
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton settingsButton;
    juce::Slider volumeSlider;
    PlaybackThumbnail playbackThumbnail;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState playbackState;

    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioWorkstationComponent)
};
