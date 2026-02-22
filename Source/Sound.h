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

class Sound {
  public:
  Sound(int id, long audioData, const juce::Image& waveForm, const std::map<std::string, std::string>& metaData);
  bool play();
  Sound applyFilter(const FilterType& filter);

  int getSoundId() const;
  long getAudioData() const;
  juce::Image getWaveForm() const;
  std::map<std::string, std::string> getMetaData() const;

private:
  int soundId;
  long audioData;
  juce::Image waveForm;
  std::map<std::string, std::string> metaData;
};