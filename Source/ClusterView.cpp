/*
  ==============================================================================

    ClusterView.cpp
    Created: 2 Mar 2026 11:54:10pm
    Author:  arkwa

  ==============================================================================
*/
//TODO give colors to the groups 
//TODO Add animation when switching cluster modes(maybe?)
//TODO inside the cluster make a menu in top to change the modes and display grouos numbers and what color is that group so give labels
//TODO apply filters for admin and guest can not?
#include "ClusterView.h"
#include "UIController.h"
ClusterView::ClusterView(ClusterEngine& e, SoundLibrary& library) : engine(e), soundLibrary(library) {}
void ClusterView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(UIController::bg));

    const auto& positions = engine.getPositions();
    const auto& clusterIds = engine.getClusterIds();

    float width = static_cast<float>(getWidth());
    float height = static_cast<float>(getHeight());

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
        if (id > maxClusterId)
            maxClusterId = id;
    }

    int clusterCount = maxClusterId + 1;

    if (clusterCount > 0)
    {
        int legendWidth = 130;
        int legendHeight = clusterCount * 20 + 16;
        int legendX = getWidth() - legendWidth - 15;
        int legendY = 12;

        g.setColour(juce::Colour(UIController::legendBg).withAlpha(0.8f));
        g.fillRoundedRectangle((float)legendX, (float)legendY,
            (float)legendWidth, (float)legendHeight, 8.0f);

        g.setColour(juce::Colour(UIController::titleText));
        g.drawRoundedRectangle((float)legendX, (float)legendY,
            (float)legendWidth, (float)legendHeight, 8.0f, 1.0f);

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
            g.drawText(getClusterLabel(c, clusterCount),
                startX + 20, y - 2, 95, 16,
                juce::Justification::left);
        }
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
        juce::Colours::purple,
        juce::Colours::blue,
        juce::Colours::green,
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

juce::String ClusterView::getClusterLabel(int clusterId, int clusterCount) const
{
    if (engine.isLengthMode())
    {
        if (clusterCount == 1)
            return "Mixed Length";

        if (clusterId == 0)
            return "Short";

        if (clusterId == clusterCount - 1)
            return "Long";

        return "Medium";
    }

    juce::String letter;
    letter << juce::juce_wchar('A' + clusterId);
    return "Pattern " + letter;
}