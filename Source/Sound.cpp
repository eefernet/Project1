/*
  ==============================================================================

    Sound.cpp
    Created: 22 Feb 2026 4:13:09pm
    Author:  eefernet

  ==============================================================================
*/
#include "Sound.h"
#include "UIController.h"

/*
* Constructor — takes a file path and immediately loads the audio data into memory.
* If the file loads successfully, it also generates a waveform thumbnail image
* that can be displayed in the UI. If the file can't be loaded (wrong format,
* missing, corrupt), the sound will be marked as invalid (isValid() returns false).
*/
Sound::Sound(int id,
    const juce::String& name,
    const juce::File& sourceFile,
    juce::AudioFormatManager& formatManager)
    : soundId(id), soundPrice("0.00"), name(name) {
    //Try to load the audio file from disk into ram
    loadFromFile(sourceFile, formatManager);
    //Only gen a waveform if we actually loaded an audio file
    if (valid)
        //thumbnail image maybe?
        generateWaveForm(100, 25);
}

/*
* This is where the actual file loading happens. We use JUCE's AudioFormatManager
* to create a reader that knows how to decode the specific file format (WAV, MP3, etc.).
* The reader gives us access to the raw PCM sample data, which we copy into our
* audioBuffer member variable.
*
* Important: This loads the ENTIRE file into memory at once. For our use case of
* ~5 sound files, this is totally fine. I dont imagine us actually putting more than 10 sounds
* on here anyways
*/
void Sound::loadFromFile(const juce::File& file, juce::AudioFormatManager& formatManager){
    //createReaderFor() looks at the file extension and header to figure out
    //what format it is, then returns an appropriate reader (WavAudioFormat,
    //MP3AudioFormat, etc.). Returns nullptr if it can't read the file.
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

    //Make sure the reader is not null before proceeding
    if (reader != nullptr){
        //store the sample rate so we know how fast to play it back
        sampleRate = reader->sampleRate;
        //allocate the buffer to match the file's channel count and length.
        //for a stereo 44100Hz file that's 3 seconds long, this would be:
        //setSize(2, 132300)  ->  2 channels, 132300 samples
        audioBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
        //read ALL samples from the file into our buffer.
        //parameters: (destBuffer, destStartSample, numSamples, readerStartSample,useLeftChannel, useRightChannel)
        //we read from the very beginning (0) and grab everything.
        reader->read(&audioBuffer, 0, (int)reader->lengthInSamples, 0, true, true);

        //Populate metadata, this is a juce thing so i decided to add it, this will be nice to have for the map
        metaData["filename"]   = file.getFileName().toStdString();
        metaData["sampleRate"] = juce::String(sampleRate).toStdString();
        metaData["channels"]   = juce::String(reader->numChannels).toStdString();
        metaData["length"]     = juce::String(reader->lengthInSamples).toStdString();
        metaData["format"]     = reader->getFormatName().toStdString();
        //Update the flag to mark it as loaded
        valid = true;
        DBG("Loaded sound: " + file.getFileName()+ " (" + juce::String(reader->lengthInSamples / sampleRate, 1) + "s)");
    }
    //Log output for dbg if the sound failed to load (probably bbecause i didnt type in the right file name)
    else{
        DBG("Failed to load: " + file.getFullPathName());
    }
}
/*
* Generates a waveform thumbnail image from the loaded audio data.
* This creates a visual representation of the sound — basically a mini waveform
* display like you'd see in a DAW or audio editor.
*
* How it works:
*   - We divide the audio buffer into "columns", one per pixel width
*   - For each column, we find the min and max sample values
*   - We draw a vertical line from min to max, centered vertically
*   - This gives us the classic waveform shape
*
* The resulting image is stored in the waveForm member and can be retrieved
* with getWaveForm() to display in a ListBox or anywhere else in the UI.
* This method was added thanks to a combbination of grok and claude
*/
void Sound::generateWaveForm(int width, int height){
    //Create a blank image to draw to
    waveForm = juce::Image(juce::Image::ARGB, width, height, true);
    juce::Graphics g(waveForm);

    //draw a background for waveform area
    g.fillAll(juce::Colour(UIController::waveformBg));
    //set a color for the waveform (green tealish color)
    g.setColour(juce::Colour(UIController::waveformFg));

    //Geta pointer to the raw sample data, for stereo files we just grabb the left channel
    auto* data = audioBuffer.getReadPointer(0);
    auto numSamples = audioBuffer.getNumSamples();
    //How many audio samples are represented by each pixel column.
    //For example, if we have 44100 samples and a 400px wide image,
    //each pixel represents about 110 samples.
    float samplesPerPixel = (float)numSamples / (float)width;

    //juce::Path wavePath;
    //vert center of the image ie silence or no sound
    float midY = height * 0.5f;

    for (int x = 0; x < width; ++x)
    {
        int startSample = (int)(x * samplesPerPixel);
        int endSample   = juce::jmin((int)((x + 1) * samplesPerPixel), numSamples);

        float minVal = 0.0f, maxVal = 0.0f;
        for (int s = startSample; s < endSample; ++s)
        {
            minVal = juce::jmin(minVal, data[s]);
            maxVal = juce::jmax(maxVal, data[s]);
        }
        g.drawVerticalLine(x, midY + minVal * midY, midY + maxVal * midY);
    }
}

bool Sound::isValid() const { return valid; }

/*
* Returns a nicely formatted duration string like "1:23" or "0:05".
* We calculate the total seconds from the number of samples and sample rate,
* then format it as minutes:seconds with zero-padding on the seconds.
*/
juce::String Sound::getDurationString() const{
    if (!valid || sampleRate == 0.0) return "0:00";
    double secs = audioBuffer.getNumSamples() / sampleRate;
    int mins = (int)secs / 60;
    int s = (int)secs % 60;
    return juce::String(mins) + ":" + juce::String(s).paddedLeft('0', 2);
}

//Getters (pretty straightforward)
int Sound::getSoundId() const { return soundId; }
juce::String Sound::getName() const { return name; }
juce::AudioBuffer<float>& Sound::getAudioBuffer() { return audioBuffer; }
double Sound::getSampleRate() const { return sampleRate; }
juce::Image Sound::getWaveForm() const { return waveForm; }
std::map<std::string, std::string> Sound::getMetaData() const { return metaData; }
juce::String Sound::getSoundPrice() const { return soundPrice; }
void Sound::setSoundPrice(const juce::String& newPrice)
{
    soundPrice = newPrice.trim();
}

void Sound::setSoundPrice(double newPrice)
{
    soundPrice = juce::String(newPrice, 2); // always 2 decimals
}

/*
* Placeholder for audio playback — currently returns false.
* Actual playback will be handled at the dashboard level using
* juce::AudioSourcePlayer and juce::AudioDeviceManager, since those
* need to be connected to the system's audio output device.
*
* TODO: We might want this method to trigger playback through a callback
* or signal to the parent component, rather than handling it directly here.
* That way the Sound class stays focused on data and the UI handles playback.
*/
bool Sound::play()
{
    //TODO: implement audio playback — see OwnerDashboardComponent for where
    //this will actually be wired up with AudioSourcePlayer
    return false;
}

/*
* Apply a filter to the sound and return the modified version.
* Right now this is a placeholder that just returns *this unchanged.
*
* TODO: Implement actual filter logic for each filter type.
* Instead of having a separate Filter.h/.cpp file, we're keeping the
* filter logic here in the Sound class since it operates directly on
* the audio buffer data.
*/
Sound Sound::applyFilter(const FilterType& filter)
{
    //TODO: apply filter logic based on filter type
    //Each filter would modify the audioBuffer data in some way
    switch (filter)
    {
        case FilterType::Pitch:
            //TODO: Pitch shifting — JUCE's built-in ResamplingAudioSource
            break;

        case FilterType::Length:
            //TODO: Time stretching — change duration without affecting pitch
            break;

        case FilterType::Effect:
            //TODO: Effects like reverb, delay, distortion, etc. Juce should have these features?
            break;
    }
    return *this;
}