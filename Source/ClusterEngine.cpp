/*
  ==============================================================================

    ClusterEngine.cpp
    Created: 3 Mar 2026 12:14:32am
    Author:  arkwa

  ==============================================================================
*/
//TODO: Upgrade number of group: fixed right now, make an algo. to make number of groups + admin can set # of groups
//TODO: A way of Re-run clustering
//TODO: Give more detail to Similarity of sound.


#include "ClusterEngine.h"

ClusterEngine::ClusterEngine(SoundLibrary& library) : soundLibrary(library)
{
}
void ClusterEngine::clusterByLength() //current: fixed group
{
    
	positions.clear();
    std::vector<std::pair<int, float>> lengths;
    int numSounds = soundLibrary.getNumSounds();
    positions.resize(numSounds);
    for (int i = 0; i < numSounds; i++)
    {
        float lengthSeconds = 0.0f;
        Sound * s = soundLibrary.getSound(i);
        if (s == nullptr || !s->isValid())
        {
            continue; 
        }
        int samp = s->getAudioBuffer().getNumSamples();
        double sr = s->getSampleRate();
        if (sr <= 0)
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
    std::sort(lengths.begin(), lengths.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b)
    {
        return a.second < b.second;
    });
    std::vector<juce::Point<float>> centers =
    {
        {0.20f, 0.25f},
        {0.50f, 0.25f},
        {0.80f, 0.25f},
        {0.35f, 0.70f},
        {0.65f, 0.70f}
    }; //centers canc change also 
    const float ja = 0.05f;
    const int N = static_cast<int>( lengths.size());
    juce::Random rn;
    for (int r = 0; r < N; r++)
    {
        int group = (r * 5) / N;
        float centerX = centers[group].x;
        float centerY = centers[group].y;
        

        int index = lengths[r].first;

        float ranX = (rn.nextFloat() * 2.0f - 1.0f) * ja;
        float ranY = (rn.nextFloat() * 2.0f - 1.0f) * ja;

        float X = centerX + ranX;
        float Y = centerY + ranY;

        X = juce::jlimit(0.0f, 1.0f, X);
        Y = juce::jlimit(0.0f, 1.0f, Y);

        positions[index] = { X, Y };

    }
}
void ClusterEngine::clusterBySimilarity() //current focusing in (length, loudness) later on: focus on sounds changes
{
    positions.clear();
    int numSounds = soundLibrary.getNumSounds();
    positions.resize(numSounds);
    struct feature {
        int i;
        float len;
        float rms;
    };
    std::vector<feature> f;
    f.reserve(numSounds);

    float minLe = std::numeric_limits<float>::max();
    float maxLe = std::numeric_limits<float>::lowest();
    float minRm = std::numeric_limits<float>::max();
    float maxRm = std::numeric_limits<float>::lowest();
    for (int j = 0; j < numSounds; j++)
    {
        float lengthSeconds = 0.0f;
        Sound* s = soundLibrary.getSound(j);
        if (s == nullptr || !s->isValid())
        {
            continue;
        }
        int samp = s->getAudioBuffer().getNumSamples();
        double sr = s->getSampleRate();
        if (sr <= 0)
        {
            continue;
        }
        lengthSeconds = static_cast<float>(samp) / static_cast<float>(sr);

        float rms = calRMS(s);

        f.push_back({ j, lengthSeconds, rms });

        minLe = std::min(minLe, lengthSeconds);
        maxLe = std::max(maxLe, lengthSeconds);
        minRm = std::min(minRm, rms);
        maxRm = std::max(maxRm, rms);
    }
    if (f.empty())
    {
        return;
    }
    float lRange = maxLe - minLe;
    float rRange = maxRm - minRm;

    const float ja = 0.05f;
    juce::Random rn;
    for (const auto& fe : f)
    {
        float x = (lRange <= 0.00001f) ? 0.5f : (fe.len - minLe) / lRange;
        float y = (rRange <= 0.00001f) ? 0.5f : (fe.rms - minRm) / rRange;
        float jx = (rn.nextFloat() * 2.0f - 1.0f) * ja;
        float jy = (rn.nextFloat() * 2.0f - 1.0f) * ja;
        x = juce::jlimit(0.0f, 1.0f, x + jx);
        y = juce::jlimit(0.0f, 1.0f, y + jy);
        positions[fe.i] = { x, y }; 
    }
}
float ClusterEngine::calRMS(Sound* s)
{
    if (s == nullptr || !s->isValid())
    {
        return 0.0f;
    }
    juce::AudioBuffer<float>& buffer = s->getAudioBuffer();
    int numCh = buffer.getNumChannels();
    int numSa = buffer.getNumSamples();

    if (numCh == 0 || numSa == 0)
    {
        return 0.0f;
    }
    double sumSq = 0.0;
    long total = 0;
    for (int ch = 0; ch < numCh; ch++)
    {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSa; i++)
        {
            sumSq += data[i] * data[i];
        }
        total += numSa;
    }
    double mean = sumSq / static_cast<double>(total);
    return static_cast<float>(std::sqrt(mean));
}
const std::vector<juce::Point<float>>& ClusterEngine::getPositions() const
{
    return positions;
}

