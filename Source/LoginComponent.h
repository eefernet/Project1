/*
  ==============================================================================

    LoginComponent.h
    Created: 10 Feb 2026 6:03:00pm
    Author:  ethan

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>

class LoginComponent : public juce::Component
{
public:
    LoginComponent();
    ~LoginComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

	//Callbacks for parent component (public facing so we can set them in the parent component)
    std::function<void(juce::String username, juce::String password)> onLogin;
    std::function<void()> onCreateAccount;

    void setMessage(const juce::String& message, juce::Colour colour);
    void clearInputs();

private:
    //UI elements for login form
    juce::Label titleLabel;
    juce::Label usernameLabel;
    juce::Label passwordLabel;

    juce::TextEditor usernameInput;
    juce::TextEditor passwordInput;

    juce::TextButton loginButton;
    juce::TextButton createAccountButton;

    juce::Label messageLabel;

    void handleLogin();
    void handleCreateAccount();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoginComponent)
};