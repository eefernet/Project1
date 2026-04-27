/*
  ==============================================================================

    ClusterView.cpp
    Created: 2 Mar 2026 11:54:10pm
    Author:  arkwa

  ==============================================================================
*/
#include "ClusterView.h"
#include "UIController.h"
ClusterView::ClusterView(ClusterEngine& e, SoundLibrary& library) : engine(e), soundLibrary(library) {}
static float chooseTickStep(float maxSeconds)
{
    if (maxSeconds <= 10.0f)
    {
        return 2.0f;
    }
    if (maxSeconds <= 30.0f) {
        return 5.0f;
    }
    if (maxSeconds <= 60.0f) { return 10.0f; }
    if (maxSeconds <= 120.0f) { return 20.0f; }
    return 30.0f;
}
void ClusterView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(UIController::bg));

    const auto& positions = engine.getPositions();
    const auto& clusterIds = engine.getClusterIds();

    float width = static_cast<float>(getWidth());
    float height = static_cast<float>(getHeight());
    float dotRadius = getDotRadius();
    g.setColour(juce::Colour(UIController::titleText));

    for (int i = 0; i < static_cast<int>(positions.size()); ++i)
    {
        float x = positions[i].x * width;
        float y = positions[i].y * height;

        int clusterId = (i < static_cast<int>(clusterIds.size())) ? clusterIds[i] : -1;
        g.setColour(getClusterColour(clusterId));
        g.fillEllipse(x - dotRadius, y - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    }
    int maxClusterId = -1;
    for (int id : clusterIds)
    {
        if (id > maxClusterId) {
            maxClusterId = id;
        }
    }

    int clusterCount = maxClusterId + 1;
    if (clusterCount > 0)
    {
        int legendWidth = 130;
        int legendHeight = clusterCount * 20 + 16;
        int legendX = getWidth() - legendWidth - 15;
        int legendY = 12;

        g.setColour(juce::Colour(UIController::legendBg).withAlpha(0.8f));
        g.fillRoundedRectangle((float)legendX, (float)legendY, (float)legendWidth, (float)legendHeight, 8.0f);

        g.setColour(juce::Colour(UIController::titleText));
        g.drawRoundedRectangle((float)legendX, (float)legendY, (float)legendWidth, (float)legendHeight, 8.0f, 1.0f);

        int startX = legendX + 10;
        int startY = legendY + 10;
        int boxSize = 12;
        int rowHeight = 20;

        for (int c = 0; c < clusterCount; ++c)
        {
            int y = startY + c * rowHeight;

            g.setColour(getClusterColour(c));
            g.fillRect(startX, y, boxSize, boxSize);
            g.setColour(juce::Colour(UIController::titleText));
            g.drawText(getClusterLabel(c, clusterCount), startX + 20, y - 2, 95, 16, juce::Justification::left);
        }
    }
    if (engine.isLengthMode())
    {
        float maxSec = 0.0f;
        for (int i = 0; i < soundLibrary.getNumSounds(); ++i)
        {
            Sound* s = soundLibrary.getSound(i);
            if (s == nullptr || !s->isValid()) {
                continue;
            }

            double sr = s->getSampleRate();
            if (sr <= 0.0) {
                continue;
            }
            float seconds = (float)s->getAudioBuffer().getNumSamples() / (float)sr;
            if (seconds > maxSec) {
                maxSec = seconds;
            }
        }

        int left = 75;
        int right = getWidth() - 45;
        int bottom = getHeight() - 70;
        int top = 90;

        g.setColour(juce::Colour(UIController::titleText));
        g.drawLine((float)left, (float)bottom, (float)right, (float)bottom);
        g.drawLine((float)left, (float)bottom, (float)left, (float)top);
        g.drawText("Estimated Pitch", left - 60, top - 28, 140, 24, juce::Justification::centred);
       
        for (int i = 0; i <= 4; ++i)
        {
            float value = i / 4.0f;
            float y = juce::jmap( value, 0.0f, 1.0f, (float)bottom, (float)top);

            g.drawLine( (float)left - 6.0f, y, (float)left + 6.0f, y);
            g.drawText(juce::String(value, 1),left - 55, (int)y - 10,45,20, juce::Justification::right);
        }

       
        if (maxSec > 0.0f)
        {
            float step = chooseTickStep(maxSec);
            for (float t = 0; t <= maxSec + .001f; t += step)
            {
                float x = juce::jmap(t, 0.0f, maxSec,  (float)left, (float)right);
                g.drawLine( x, (float)bottom - 6, x, (float)bottom + 6);
                g.drawText( juce::String(t, 0) + "s", (int)x - 25, bottom + 12, 50, 20, juce::Justification::centred);
            }
        }
        g.drawText( "Length (seconds)", left, bottom + 38, right - left, 24, juce::Justification::centred);
    
    }
    else
    {
        int left = 75;
        int right = getWidth() - 90;
        int bottom = getHeight() - 70;
        int top = 90;

        g.setColour(juce::Colour(UIController::titleText));
        g.drawLine((float)left, (float)bottom, (float)right, (float)bottom);
        g.drawLine((float)left, (float)bottom, (float)left, (float)top);
        g.drawText("Loudness (RMS)", left - 60, top - 28, 140, 24, juce::Justification::centred);
        for (int i = 0; i <= 4; ++i)
        {
            float value = i / 4.0f;

            float y = juce::jmap( value, 0.0f, 1.0f, (float)bottom, (float)top);

            g.drawLine( (float)left - 6.0f,  y,  (float)left + 6.0f,  y);

            g.drawText( juce::String(value, 1), left - 55, (int)y - 10, 45, 20, juce::Justification::right);
        }
        for (int i = 0; i <= 4; ++i)
        {
            float value = i / 4.0f;

            float x = juce::jmap( value,  0.0f, 1.0f, (float)left, (float)right);

            g.drawLine(  x, (float)bottom - 6,  x,  (float)bottom + 6);

            g.drawText( juce::String(value, 1),  (int)x - 25,  bottom + 12,  50,  20, juce::Justification::centred);
        }
        g.drawText( "Brightness",  left, bottom + 38, right - left, 24, juce::Justification::centred);
    }
    if (hoveredDotIndex >= 0 && hoveredDotIndex < static_cast<int>(positions.size()))
    {
        Sound* hoveredSound = soundLibrary.getSound(hoveredDotIndex);

        if (hoveredSound != nullptr)
        {
            juce::String songName = hoveredSound->getName();

            float x = positions[hoveredDotIndex].x * width;
            float y = positions[hoveredDotIndex].y * height;

            juce::Font font(14.0f);
            g.setFont(font);

            int padding = 6;
            int textWidth = font.getStringWidth(songName);
            int boxWidth = textWidth + padding * 2;
            int boxHeight = 24;

            int boxX = static_cast<int>(x - boxWidth / 2.0f);
            int boxY = static_cast<int>(y - dotRadius - boxHeight - 8);

            boxX = juce::jlimit(0, getWidth() - boxWidth, boxX);
            boxY = juce::jmax(0, boxY);

            g.setColour(juce::Colour(UIController::tooltipBg));
            g.fillRoundedRectangle((float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight, 6.0f);

            g.setColour(juce::Colour(UIController::tooltipOutline));
            g.drawRoundedRectangle((float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight, 6.0f, 1.0f);

            g.drawText(songName, boxX, boxY, boxWidth, boxHeight, juce::Justification::centred);
        }
    }
}

void ClusterView::resized()
{
}

int ClusterView::getDotAtPosition(juce::Point<float> mousePos) const
{
    const auto& positions = engine.getPositions();

    float width = static_cast<float>(getWidth());
    float height = static_cast<float>(getHeight());
    float dotRadius = getDotRadius();
    for (int i = 0; i < static_cast<int>(positions.size()); ++i)
    {
        float x = positions[i].x * width;
        float y = positions[i].y * height;

        float dx = mousePos.x - x;
        float dy = mousePos.y - y;
        float distSq = dx * dx + dy * dy;

        if (distSq <= dotRadius * dotRadius)
            return i;
    }

    return -1;
}

void ClusterView::mouseMove(const juce::MouseEvent& event)
{
    int newHoveredDot = getDotAtPosition(event.position);

    if (newHoveredDot != hoveredDotIndex)
    {
        hoveredDotIndex = newHoveredDot;
        repaint();
    }
}

void ClusterView::mouseExit(const juce::MouseEvent&)
{
    if (hoveredDotIndex != -1)
    {
        hoveredDotIndex = -1;
        repaint();
    }
}
juce::Colour ClusterView::getClusterColour(int clusterId) const
{
    static const std::vector<juce::Colour> colours =
    {
        juce::Colours::magenta,
        juce::Colours::aqua,
        juce::Colours::limegreen,
        juce::Colours::orange,
        juce::Colours::red,
        juce::Colours::yellow,
        juce::Colours::pink,
        juce::Colours::cyan
    };

    if (clusterId < 0)
        return juce::Colour(UIController::titleText);

    return colours[clusterId % colours.size()];
}
void ClusterView::mouseDown(const juce::MouseEvent& event)
{
    int clickedDot = getDotAtPosition(event.position);
    if (clickedDot >= 0 && onDotClicked)
    {
        onDotClicked(clickedDot);
    }
}
float ClusterView::getDotRadius() const
{
    int numSounds = static_cast<int>(engine.getPositions().size());
    if (numSounds <= 0)
    {
        return 4.0f;
    }
    return juce::jlimit(4.0f, 12.0f, 25.0f / std::sqrt(static_cast<float>(numSounds)));
}

juce::String ClusterView::getClusterLabel(int clusterId, int clusterCount) const
{
    if (engine.isLengthMode())
    {
        juce::String letter;
        letter << juce::juce_wchar('A' + clusterId);
        return "Group " + letter;
    }

    juce::String letter;
    letter << juce::juce_wchar('A' + clusterId);
    return "Pattern " + letter;
}
