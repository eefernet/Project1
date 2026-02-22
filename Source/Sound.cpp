/*
  ==============================================================================

    Sound.cpp
    Created: 22 Feb 2026 4:13:09pm
    Author:  eefernet

  ==============================================================================
*/

#include "Sound.h"

Sound::Sound(int id, long audioData, const juce::Image& waveForm, const std::map<std::string, std::string>& metaData)
    : soundId(id), audioData(audioData), waveForm(waveForm), metaData(metaData) {}

bool Sound::play() {
  //TODO: implement audio playback loop here not sure if it has to be on a seperate thread?
  return false;
}

//Apply a filter, filter logic will be added soon
Sound Sound::applyFilter(const FilterType& filter) {
  //TODO: apply filter logic based on filter type, might just feed back into play to listen to filter
  switch (filter) {
    //Im thinking instead of having a seperate filter h file and cpp file we just shove that logic here
    case FilterType::Pitch:
      break;
    case FilterType::Length:
      break;
    case FilterType::Effect:
      break;
  }
  return *this;
}

//Getters and setters
int Sound::getSoundId() const { return soundId; }
long Sound::getAudioData() const { return audioData; }
juce::Image Sound::getWaveForm() const { return waveForm; }
std::map<std::string, std::string> Sound::getMetaData() const { return metaData; }