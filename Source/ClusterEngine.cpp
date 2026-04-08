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
    std::vector<std::pair<int, float>> lengths;
    int numSounds = soundLibrary.getNumSounds();
    positions.resize(numSounds);
    clusterIds.resize(numSounds, -1);

    for (int i = 0; i < numSounds; i++)
    {
        float lengthSeconds = 0.0f;
        Sound* s = soundLibrary.getSound(i);
        if (s == nullptr || !s->isValid())
        {
            continue;
        }

        int samp = s->getAudioBuffer().getNumSamples();
        double sr = s->getSampleRate();
        if (sr <= 0.0)
        {
            continue;
        }

        lengthSeconds = static_cast<float>(samp) / static_cast<float>(sr);
        lengths.push_back({ i, lengthSeconds });
    }

    if (lengths.empty())
    {
        return;
    }

    std::sort(lengths.begin(), lengths.end(),
        [](const std::pair<int, float>& a, const std::pair<int, float>& b)
        {
            return a.second < b.second;
        });

    float totalGap = 0.0f;
    int gapCount = 0;

    for (size_t i = 1; i < lengths.size(); i++)
    {
        float gap = lengths[i].second - lengths[i - 1].second;
        totalGap += gap;
        gapCount++;
    }
    float avgGap = (gapCount > 0) ? (totalGap / static_cast<float>(gapCount)) : 0.0f;
    float t = avgGap * 1.5f;

    if (t < 0.25f)
    {
        t = 0.25f;
    }
    std::vector<std::vector<int>> clusters;
    clusters.push_back({ lengths[0].first });
    for (size_t i = 1; i < lengths.size(); i++)
    {
        float gap = lengths[i].second - lengths[i - 1].second;
        if (gap > t)
        {
            clusters.push_back({});
        }
        clusters.back().push_back(lengths[i].first);

    }
    juce::Random rn;
    const float amount = 0.05f;
    int clusterCount = static_cast<int>(clusters.size());
    for (int p = 0; p < clusterCount; p++)
    {
        for (int soundIndex : clusters[p])
        {
            clusterIds[soundIndex] = p;
        }
    }
    for (int c = 0; c < clusterCount; c++)
    {
        float centerX;
        float centerY;

        if (clusterCount == 1)
        {
            centerX = 0.5f;
            centerY = 0.5f;
        }
        else
        {
            centerX = 0.15f + (0.70f * c) / static_cast<float>(clusterCount - 1);
            centerY = (c % 2 == 0) ? 0.35f : 0.65f;
        }
        for (int soundIn : clusters[c])
        {
            float ranX = (rn.nextFloat() * 2.0f - 1.0f) * amount;
            float ranY = (rn.nextFloat() * 2.0f - 1.0f) * amount;
            float x = juce::jlimit(0.0f, 1.0f, centerX + ranX);
            float y = juce::jlimit(0.0f, 1.0f, centerY + ranY);

            positions[soundIn] = { x, y };
        }
    }
}
std::vector<float> ClusterEngine::extractWaveformFeature(Sound* s, int target)
{
    std::vector<float> feature;
    if (s == nullptr || !s->isValid() || target <= 0)
    {
        return feature;
    }
    juce::AudioBuffer<float>& buffer = s->getAudioBuffer();
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0)
    {
        return feature;
    }
    feature.resize(target, 0.0f);
    for (int i = 0; i < target; ++i)
    {
        int start = (i * numSamples) / target;
        int end = ((i + 1) * numSamples) / target;

        if (end <= start)
        {
            end = start + 1;
        }
        float sum = 0.0f;
        int count = 0;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* data = buffer.getReadPointer(ch);
            for (int sInd = start; sInd < end && sInd < numSamples; ++sInd)
            {
                sum += std::abs(data[sInd]);
                count++;
            }
        }
        feature[i] = (count > 0) ? (sum / static_cast<float>(count)) : 0.0f;
    }

    return feature;
}
float ClusterEngine::waveformDistance(const std::vector<float>& a, const std::vector<float>& b)
{
    if (a.size() != b.size() || a.empty())
    {
        return std::numeric_limits<float>::max();
    }
    float sum = 0.0f;
    for (size_t i = 0; i < a.size(); ++i)
    {
        float dif = a[i] - b[i];
        sum += dif * dif;
    }
    return std::sqrt(sum);
}
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
    struct waveFeature {
        int index;
        std::vector<float> feature;
    };

    std::vector<waveFeature> f;
    f.reserve(numSounds);

    for (int j = 0; j < numSounds; j++)
    {
        Sound* s = soundLibrary.getSound(j);
        if (s == nullptr || !s->isValid())
        {
            continue;
        }

        std::vector<float> waveform = extractWaveformFeature(s, 64);
        if (!waveform.empty())
        {
            f.push_back({ j , waveform });
        }
    }
    if (f.empty())
    {
        return;
    }
    std::vector<std::vector<int>> clusters;
    std::vector<bool> used(f.size(), false);

    float th = 0.35f;
    for (size_t i = 0; i < f.size(); i++)
    {
        if (used[i]) {
            continue;
        }
        clusters.push_back({});
        clusters.back().push_back(f[i].index);
        used[i] = true;
        for (size_t j = i + 1; j < f.size(); j++)
        {
            if (used[j]) {
                continue;
            }
            float dist = waveformDistance(f[i].feature, f[j].feature);
            if (dist < th)
            {
                clusters.back().push_back(f[j].index);
                used[j] = true;
            }
        }
    }

    juce::Random rn;
    const float jAmount = 0.05f;
    int clusterCount = static_cast<int>(clusters.size());
    for (int c = 0; c < clusterCount; ++c)
    {
        for (int soundIndex : clusters[c])
        {
            clusterIds[soundIndex] = c;
        }
    }

    for (int c = 0; c < clusterCount; ++c)
    {
        float centerX;
        float centerY;

        if (clusterCount == 1)
        {
            centerX = 0.5f;
            centerY = 0.5f;
        }
        else
        {
            centerX = 0.15f + (0.70f * c) / static_cast<float>(clusterCount - 1);
            centerY = (c % 2 == 0) ? 0.35f : 0.65f;
        }

        for (int soundIndex : clusters[c])
        {
            float ranX = (rn.nextFloat() * 2.0f - 1.0f) * jAmount;
            float ranY = (rn.nextFloat() * 2.0f - 1.0f) * jAmount;

            float x = juce::jlimit(0.0f, 1.0f, centerX + ranX);
            float y = juce::jlimit(0.0f, 1.0f, centerY + ranY);

            positions[soundIndex] = { x, y };
        }
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

