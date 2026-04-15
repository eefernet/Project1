#pragma once

#include <JuceHeader.h>

/*
 * Central UI theme / LookAndFeel. Current style: flat, minimal, terminal-inspired
 * This is much easier to change here rather than changing everything in each file.
 * We can change something once here and it changes everywhere. Its like a skin we put on top of it.
 * Swap the palette/fonts here to retheme the whole app without touching components.
 * Applied globally via juce::LookAndFeel::setDefaultLookAndFeel in MainComponent.
 * This was ripped straight from StackOverflow so thanks random internet dude
 */
class UIController : public juce::LookAndFeel_V4
{
public:
    // Palette
    static constexpr juce::uint32 bg          = 0xff0a0f0a; // near-black with green tint
    static constexpr juce::uint32 bgRaised    = 0xff101810;
    static constexpr juce::uint32 bgHover     = 0xff162016;
    static constexpr juce::uint32 border      = 0xff2a3a2a;
    static constexpr juce::uint32 borderHot   = 0xff39ff14; // phosphor green
    static constexpr juce::uint32 text        = 0xffc8f7c8;
    static constexpr juce::uint32 textDim     = 0xff6fa36f;
    static constexpr juce::uint32 accent      = 0xff39ff14;
    static constexpr juce::uint32 accentAmber = 0xffffb000;
    static constexpr juce::uint32 danger      = 0xffff5555;

    UIController()
    {
        // Leave default sans-serif alone so the OS can fall back to emoji fonts.
        // Individual widgets opt into monospaced via getTextButtonFont / getLabelFont.

        // Window / panels
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(bg));

        // Labels
        setColour(juce::Label::textColourId,            juce::Colour(text));
        setColour(juce::Label::backgroundColourId,      juce::Colours::transparentBlack);
        setColour(juce::Label::outlineColourId,         juce::Colours::transparentBlack);

        // Text editors
        setColour(juce::TextEditor::backgroundColourId, juce::Colour(bgRaised));
        setColour(juce::TextEditor::textColourId,       juce::Colour(text));
        setColour(juce::TextEditor::highlightColourId,  juce::Colour(accent).withAlpha(0.3f));
        setColour(juce::TextEditor::highlightedTextColourId, juce::Colour(text));
        setColour(juce::TextEditor::outlineColourId,         juce::Colour(border));
        setColour(juce::TextEditor::focusedOutlineColourId,  juce::Colour(accent));
        setColour(juce::CaretComponent::caretColourId,       juce::Colour(accent));

        // Buttons
        setColour(juce::TextButton::buttonColourId,      juce::Colour(bgRaised));
        setColour(juce::TextButton::buttonOnColourId,    juce::Colour(accent).withAlpha(0.15f));
        setColour(juce::TextButton::textColourOffId,     juce::Colour(text));
        setColour(juce::TextButton::textColourOnId,      juce::Colour(accent));

        // ComboBox
        setColour(juce::ComboBox::backgroundColourId,    juce::Colour(bgRaised));
        setColour(juce::ComboBox::textColourId,          juce::Colour(text));
        setColour(juce::ComboBox::outlineColourId,       juce::Colour(border));
        setColour(juce::ComboBox::arrowColourId,         juce::Colour(accent));
        setColour(juce::ComboBox::buttonColourId,        juce::Colour(bgRaised));

        // Popup menu
        setColour(juce::PopupMenu::backgroundColourId,         juce::Colour(bgRaised));
        setColour(juce::PopupMenu::textColourId,               juce::Colour(text));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(accent).withAlpha(0.2f));
        setColour(juce::PopupMenu::highlightedTextColourId,    juce::Colour(accent));

        // ListBox
        setColour(juce::ListBox::backgroundColourId,     juce::Colour(bg));
        setColour(juce::ListBox::outlineColourId,        juce::Colour(border));
        setColour(juce::ListBox::textColourId,           juce::Colour(text));

        // Scrollbars
        setColour(juce::ScrollBar::backgroundColourId,   juce::Colour(bg));
        setColour(juce::ScrollBar::thumbColourId,        juce::Colour(border));
        setColour(juce::ScrollBar::trackColourId,        juce::Colour(bgRaised));

        // Sliders
        setColour(juce::Slider::backgroundColourId,      juce::Colour(bgRaised));
        setColour(juce::Slider::trackColourId,           juce::Colour(border));
        setColour(juce::Slider::thumbColourId,           juce::Colour(accent));
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(accent));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(border));
        setColour(juce::Slider::textBoxTextColourId,     juce::Colour(text));
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(bgRaised));
        setColour(juce::Slider::textBoxOutlineColourId,  juce::Colour(border));

        // ToggleButton
        setColour(juce::ToggleButton::textColourId,      juce::Colour(text));
        setColour(juce::ToggleButton::tickColourId,      juce::Colour(accent));
        setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(textDim));

        // Alerts
        setColour(juce::AlertWindow::backgroundColourId, juce::Colour(bgRaised));
        setColour(juce::AlertWindow::textColourId,       juce::Colour(text));
        setColour(juce::AlertWindow::outlineColourId,    juce::Colour(accent));
    }

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        // Monospaced for the terminal look — font fallback handles emoji glyphs.
        return juce::Font(juce::Font::getDefaultMonospacedFontName(),
                          juce::jmax(14.0f, buttonHeight * 0.45f),
                          juce::Font::plain);
    }

    juce::Font getLabelFont(juce::Label& l) override
    {
        auto h = l.getHeight() > 0 ? l.getHeight() : 20;
        return juce::Font(juce::Font::getDefaultMonospacedFontName(),
                          juce::jlimit(12.0f, 18.0f, h * 0.55f),
                          juce::Font::plain);
    }

    // Flat buttons with a 1px border. Hover/press = brighter border + subtle fill.
    void drawButtonBackground(juce::Graphics& g, juce::Button& b,
                              const juce::Colour& /*base*/,
                              bool isHover, bool isDown) override
    {
        auto r = b.getLocalBounds().toFloat().reduced(0.5f);
        auto fill = juce::Colour(bgRaised);
        auto line = juce::Colour(border);

        if (b.getToggleState())       { fill = juce::Colour(accent).withAlpha(0.15f); line = juce::Colour(accent); }
        if (isHover)                  { fill = juce::Colour(bgHover); line = juce::Colour(accent).withAlpha(0.7f); }
        if (isDown)                   { fill = juce::Colour(accent).withAlpha(0.25f); line = juce::Colour(accent); }

        g.setColour(fill);
        g.fillRect(r);
        g.setColour(line);
        g.drawRect(r, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& b,
                        bool isHover, bool isDown) override
    {
        // Base text colour comes from the button's own palette so individual
        // buttons (e.g. Logout) can opt into a custom colour like red.
        auto c = b.findColour(juce::TextButton::textColourOffId);
        if (b.getToggleState() || isDown) c = juce::Colour(accent);
        else if (isHover)                 c = c.brighter(0.2f);

        g.setColour(c);
        g.setFont(getTextButtonFont(b, b.getHeight()));

        auto txt = "> " + b.getButtonText();
        g.drawFittedText(txt, b.getLocalBounds().reduced(10, 0),
                         juce::Justification::centredLeft, 1);
    }

    // Flat text editor outlines
    void fillTextEditorBackground(juce::Graphics& g, int w, int h, juce::TextEditor&) override
    {
        g.setColour(juce::Colour(bgRaised));
        g.fillRect(0, 0, w, h);
    }

    void drawTextEditorOutline(juce::Graphics& g, int w, int h, juce::TextEditor& ed) override
    {
        g.setColour(ed.hasKeyboardFocus(true) ? juce::Colour(accent)
                                              : juce::Colour(border));
        g.drawRect(0, 0, w, h, 1);
    }
};
