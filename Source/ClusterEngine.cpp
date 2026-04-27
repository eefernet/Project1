/*
  ==============================================================================

    ClusterEngine.cpp
    Created: 3 Mar 2026 12:14:32am
    Author:  arkwa

  ==============================================================================
*/



#include "ClusterEngine.h"

ClusterEngine::ClusterEngine(SoundLibrary& library) : soundLibrary(library)
{
}
void ClusterEngine::clusterByLength()
{
    positions.clear();
    clusterIds.clear();
    lengthMode = true;

    int numSounds = soundLibrary.getNumSounds();
    positions.resize(numSounds);
    clusterIds.resize(numSounds, -1);

    struct SoundPoint {
        int index; 
        float lengthSeconds;
        float pitchHz;
    };

    std::vector<SoundPoint> soundPoints;
    float minLength = std::numeric_limits<float>::max();
    float maxLength = 0.0f;
    float minPitch = std::numeric_limits<float>::max();
    float maxPitch = 0.0f;

    for (int i = 0; i < numSounds; ++i)
    {
        Sound* s = soundLibrary.getSound(i);

        if (s == nullptr || !s->isValid()) {
            continue;
        }

        auto& buffer = s->getAudioBuffer();
        double sr = s->getSampleRate();

        if (sr <= 0.0 || buffer.getNumSamples() <= 0) {
            continue;
        }

        float lengthSeconds = static_cast<float>(buffer.getNumSamples() / sr);

        int crossings = 0;
        int count = 0;

        for (int ch = 0; ch < buffer.getNumChannels(); ch++)
        {
            const float* data = buffer.getReadPointer(ch);

            for (int sample = 1; sample < buffer.getNumSamples(); sample++)
            {
                bool crossed = (data[sample - 1] <= 0.0f && data[sample] > 0.0f) ||
                 (data[sample - 1] >= 0.0f && data[sample] < 0.0f);

                if (crossed) {
                    crossings++;
                }

                count++;
            }
        }

        float pitchHz = 0.0f;

        if (count > 0)
        {
            float seconds = static_cast<float>(buffer.getNumSamples() / sr);
            pitchHz = (crossings / 2.0f) / seconds;
        }

        soundPoints.push_back({ i, lengthSeconds, pitchHz });

        minLength = std::min(minLength, lengthSeconds);
        maxLength = std::max(maxLength, lengthSeconds);

        if (pitchHz > 0.0f)
        {
            minPitch = std::min(minPitch, pitchHz);
            maxPitch = std::max(maxPitch, pitchHz);
        }
    }

    if (soundPoints.empty() || maxLength <= minLength) {
        return;
    }

    int currentCluster = 0;
    float threshold = 0.16f;

    std::vector<float> normX(soundPoints.size(), 0.0f);
    std::vector<float> normY(soundPoints.size(), 0.5f);

    for (size_t i = 0; i < soundPoints.size(); i++)
    {
        normX[i] = (soundPoints[i].lengthSeconds - minLength) / (maxLength - minLength);

        if (maxPitch > minPitch && soundPoints[i].pitchHz > 0.0f)
        {
            normY[i] = (soundPoints[i].pitchHz - minPitch) / (maxPitch - minPitch);
        }
    }

    for (size_t i = 0; i < soundPoints.size(); i++)
    {
        if (clusterIds[soundPoints[i].index] != -1) {
            continue;
        }

        std::vector<size_t> stack;
        stack.push_back(i);
        clusterIds[soundPoints[i].index] = currentCluster;

        while (!stack.empty())
        {
            size_t current = stack.back();
            stack.pop_back();

            for (size_t j = 0; j < soundPoints.size(); j++)
            {
                if (clusterIds[soundPoints[j].index] != -1) {
                    continue;
                }
                float dx = normX[current] - normX[j];
                float dy = normY[current] - normY[j];
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < threshold)
                {
                    clusterIds[soundPoints[j].index] = currentCluster;
                    stack.push_back(j);
                }
            }
        }

        currentCluster++;
    }
    juce::Random rn;
    for (const auto& point : soundPoints)
    {
        float x = (point.lengthSeconds - minLength) / (maxLength - minLength);
        float y = 0.5f;

        if (maxPitch > minPitch && point.pitchHz > 0.0f) {
            y = (point.pitchHz - minPitch) / (maxPitch - minPitch);
        }

        x = juce::jmap(x, 0.0f, 1.0f, 0.12f, 0.97f);
        y = juce::jmap(y, 0.0f, 1.0f, 0.80f, 0.20f);

        float jitter = 0.0f;

        x += (rn.nextFloat() * 2.0f - 1.0f) * jitter;
        y += (rn.nextFloat() * 2.0f - 1.0f) * jitter;
        x = juce::jlimit(0.12f, 0.97f, x);
        y = juce::jlimit(0.20f, 0.80f, y);

        positions[point.index] = { x, y };
    }
}
//std::vector<float> ClusterEngine::extractWaveformFeature(Sound* s, int target)
//{
//    std::vector<float> feature;
//    if (s == nullptr || !s->isValid() || target <= 0)
//    {
//        return feature;
//    }
//    juce::AudioBuffer<float>& buffer = s->getAudioBuffer();
//    int numChannels = buffer.getNumChannels();
//    int numSamples = buffer.getNumSamples();
//
//    if (numChannels == 0 || numSamples == 0)
//    {
//        return feature;
//    }
//    feature.resize(target, 0.0f);
//    for (int i = 0; i < target; ++i)
//    {
//        int start = (i * numSamples) / target;
//        int end = ((i + 1) * numSamples) / target;
//
//        if (end <= start)
//        {
//            end = start + 1;
//        }
//        float sum = 0.0f;
//        int count = 0;
//
//        for (int ch = 0; ch < numChannels; ++ch)
//        {
//            const float* data = buffer.getReadPointer(ch);
//            for (int sInd = start; sInd < end && sInd < numSamples; ++sInd)
//            {
//                sum += std::abs(data[sInd]);
//                count++;
//            }
//        }
//        feature[i] = (count > 0) ? (sum / static_cast<float>(count)) : 0.0f;
//    }
//
//    return feature;
//}
//float ClusterEngine::waveformDistance(const std::vector<float>& a, const std::vector<float>& b)
//{
//    if (a.size() != b.size() || a.empty())
//    {
//        return std::numeric_limits<float>::max();
//    }
//    float sum = 0.0f;
//    for (size_t i = 0; i < a.size(); ++i)
//    {
//        float dif = a[i] - b[i];
//        sum += dif * dif;
//    }
//    return std::sqrt(sum);
//}
const std::vector<juce::Point<float>>& ClusterEngine::getPositions() const
{
    return positions;
}

void ClusterEngine::clusterBySimilarity()
{
    positions.clear();
    clusterIds.clear();
    lengthMode = false;
    int numSounds = soundLibrary.getNumSounds();
    positions.resize(numSounds);
    clusterIds.resize(numSounds, -1);
    struct SoundPoint
    {
        int index;
        float brightness;
        float loudness;
    };
    std::vector<SoundPoint> soundPoints;
    float minBrightness = std::numeric_limits<float>::max();
    float maxBrightness = 0.0f;
    float minLoudness = std::numeric_limits<float>::max();
    float maxLoudness = 0.0f;
    for (int i = 0; i < numSounds; ++i)
    {
        Sound* s = soundLibrary.getSound(i);

        if (s == nullptr || !s->isValid()) {
            continue;
        }

        auto& buffer = s->getAudioBuffer();
        if (buffer.getNumSamples() <= 0 || buffer.getNumChannels() <= 0) {
            continue;
        }

        float totalAbs = 0.0f;
        float totalSquared = 0.0f;
        int count = 0;
        for (int ch = 0; ch < buffer.getNumChannels(); ch++)
        {
            const float* data = buffer.getReadPointer(ch);

            for (int sample = 0; sample < buffer.getNumSamples(); sample++)
            {
                float value = data[sample];
                totalAbs += std::abs(value);
                totalSquared += value * value;
                count++;
            }
        }

        if (count <= 0) {
            continue;
        }

        float avgAmplitude = totalAbs / static_cast<float>(count);
        float rms = std::sqrt(totalSquared / static_cast<float>(count));
        int crossings = 0;
        for (int ch = 0; ch < buffer.getNumChannels(); ch++)
        {
            const float* data = buffer.getReadPointer(ch);
            for (int sample = 1; sample < buffer.getNumSamples(); ++sample)
            {
                bool crossed =
                    (data[sample - 1] <= 0.0f && data[sample] > 0.0f) ||
                    (data[sample - 1] >= 0.0f && data[sample] < 0.0f);
                if (crossed) {
                    crossings++;
                }
            }
        }

        float brightness = static_cast<float>(crossings) / static_cast<float>(count);
        float loudness = rms;
        soundPoints.push_back({ i, brightness, loudness });
        minBrightness = std::min(minBrightness, brightness);
        maxBrightness = std::max(maxBrightness, brightness);
        minLoudness = std::min(minLoudness, loudness);
        maxLoudness = std::max(maxLoudness, loudness);
    }

    if (soundPoints.empty() || maxBrightness <= minBrightness || maxLoudness <= minLoudness)
    {
        return;
    }

    std::vector<float> normX(soundPoints.size(), 0.0f);
    std::vector<float> normY(soundPoints.size(), 0.0f);
    for (size_t i = 0; i < soundPoints.size(); ++i)
    {
        normX[i] = (soundPoints[i].brightness - minBrightness) / (maxBrightness - minBrightness);

        normY[i] = (soundPoints[i].loudness - minLoudness) / (maxLoudness - minLoudness);
    }

    int currentCluster = 0;
    float threshold = 0.16f;

    for (size_t i = 0; i < soundPoints.size(); i++)
    {
        if (clusterIds[soundPoints[i].index] != -1) {
            continue;
        }

        std::vector<size_t> stack;
        stack.push_back(i);
        clusterIds[soundPoints[i].index] = currentCluster;

        while (!stack.empty())
        {
            size_t current = stack.back();
            stack.pop_back();
            for (size_t j = 0; j < soundPoints.size(); j++)
            {
                if (clusterIds[soundPoints[j].index] != -1) {
                    continue;
                }

                float dx = normX[current] - normX[j];
                float dy = normY[current] - normY[j];
                float distance = std::sqrt(dx * dx + dy * dy);
                if (distance < threshold)
                {
                    clusterIds[soundPoints[j].index] = currentCluster;
                    stack.push_back(j);
                }
            }
        }

        currentCluster++;
    }

    for (size_t i = 0; i < soundPoints.size(); ++i)
    {
        float x = normX[i];
        float y = normY[i];

        x = juce::jmap(x, 0.0f, 1.0f, 0.12f, 0.97f);
        y = juce::jmap(y, 0.0f, 1.0f, 0.80f, 0.20f);

        x = juce::jlimit(0.12f, 0.97f, x);
        y = juce::jlimit(0.20f, 0.80f, y);

        positions[soundPoints[i].index] = { x, y };
    }
}

const std::vector<int>& ClusterEngine::getClusterIds() const
{
    return clusterIds;
}
bool ClusterEngine::isLengthMode() const
{
    return lengthMode;
}



