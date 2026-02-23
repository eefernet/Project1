/*
  ==============================================================================

    Sound.h
    Created: 22 Feb 2026 4:13:09pm
    Author:  eefernet

  ==============================================================================
*/

#pragma once
#include "Filter.h"
#include <JuceHeader.h>
#include <map>
#include <string>
/*
* The Sound class represents a single audio file that has been loaded from disk.
* It holds the actual audio sample data in a juce::AudioBuffer, along with a
* generated waveform thumbnail image and metadata extracted from the file.
*
* Previously this class used a `long audioData` placeholder — that has been replaced
* with a proper juce::AudioBuffer<float> which stores the actual PCM sample data
* in memory. This means the entire file is loaded into RAM, which is fine for short
* sounds (a few seconds to a couple minutes) which is all we are trying to accomplish.
*
* Usage:
*   juce::AudioFormatManager formatManager;
*   formatManager.registerBasicFormats();
*   Sound mySound(1, "kick", juce::File("/path/to/kick.wav"), formatManager);
*   if (mySound.isValid()) { ... }
*/
class Sound {
  /*
    * Constructor that loads an audio file from disk.
    * @param id             - Unique identifier for this sound
    * @param name           - Display name (usually the filename without extension)
    * @param sourceFile     - The actual file on disk to load (wav, mp3, aiff, flac, ogg)
    * @param formatManager  - A reference to the AudioFormatManager that knows how to
    *                         decode various audio formats. This must have had
    *                         registerBasicFormats() called on it before passing it here.
    */
  public:
  Sound(int id, const juce::String& name, const juce::File& sourceFile, juce::AudioFormatManager& formatManager);
  /*
    * Returns true if the audio file was loaded successfully.
    * If this returns false, the file was either missing, corrupt, or in an
    * unsupported format. You should check this before trying to use the sound.
    */
  bool isValid() const;
  /*
    * Plays the sound. Currently returns false as a placeholder — actual playback
    * will be handled by the dashboard component using juce::AudioSourcePlayer.
    * TODO: implement audio playback, possibly on a separate thread so we don't
    * block the UI. The audio callback thread in JUCE handles this for us if we
    * use AudioSourcePlayer properly.
    */
  bool play();
  /*
    * Apply a filter to the sound (pitch, length, effect).
    * Returns a modified copy of this Sound with the filter applied.
    * TODO: implement actual filter logic — right now this just returns *this unchanged.
    */
  Sound applyFilter(const FilterType& filter);
  //Return soundID and name
  int getSoundId() const;
  juce::String getName() const;
  /*
    * Returns a reference to the raw audio sample buffer.
    * This contains the actual float PCM data that was read from the file.
    * You can pass this to an AudioSource or AudioSourcePlayer for playback.
    * NOTE: This returns a non-const reference so playback code can read from it.
    */
  juce::AudioBuffer<float>& getAudioBuffer();
  //Returns sample rate
  double getSampleRate() const;
  /*
    * Returns a pre-generated waveform thumbnail image.
    * This is an ARGB image that shows the amplitude of the audio over time,
    * generated automatically when the file is loaded. Useful for displaying
    * in a list or detail view so the user can see the shape of the sound.
    */
  juce::Image getWaveForm() const;
  /*
    * Returns metadata extracted from the audio file.
    * Includes keys like: "filename", "sampleRate", "channels", "length", "format"
    * This is a simple string-to-string map so it's easy to display in the UI.
    */
  std::map<std::string, std::string> getMetaData() const;
  /*
    * Returns a human-readable duration string like "1:23" or "0:05".
    * Calculated from the number of samples divided by the sample rate.
    */
  juce::String getDurationString() const;

private:
  /*
    * Internal method that actually reads the audio file from disk into our buffer.
    * Uses the AudioFormatManager to create an appropriate reader (WAV, MP3, etc.),
    * then reads ALL samples into audioBuffer. Also populates the metaData map.
    *
    * @param file           - The file to load
    * @param formatManager  - The format manager that can create readers for this file type
    */
  void loadFromFile(const juce::File& file, juce::AudioFormatManager& formatManager);
  /*
    * Internal method that generates a waveform thumbnail image from the loaded audio data.
    * Scans through the audio buffer and draws the min/max amplitude for each pixel column,
    * producing a visual representation of the sound's shape.
    *
    * @param width  - Width of the generated image in pixels
    * @param height - Height of the generated image in pixels
    */
  void generateWaveForm(int width, int height);

  //Unique soundID
  int soundId;
  //Human readable display name
  juce::String name;
  /*
    * The actual audio sample data stored in memory.
    * This replaces the old `long audioData` placeholder.
    * Contains float samples for each channel (mono = 1 channel, stereo = 2 channels).
    * The buffer size equals the total number of samples in the file.
    */
  juce::AudioBuffer<float> audioBuffer;
  double sampleRate = 0.0;
  juce::Image waveForm;
  std::map<std::string, std::string> metaData;
  bool valid = false;
};