/*
  ==============================================================================

    LoginComponent.cpp
    Created: 10 Feb 2026 6:02:49pm
    Author:  ethan

  ==============================================================================
*/

#include "LoginComponent.h"

/*
* THis is the view controller for the login screen, it will handle user input and display messages 
* based on login success or failure. It will also provide options for guest access and account creation.
*/
LoginComponent::LoginComponent()
{
    //Title
    titleLabel.setText("Sound App Login", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    //Username Label
    usernameLabel.setText("Username:", juce::dontSendNotification);
    usernameLabel.setJustificationType(juce::Justification::right);
    usernameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(usernameLabel);

    //Username Input
    usernameInput.setTextToShowWhenEmpty("Enter username", juce::Colours::grey);
    usernameInput.setFont(juce::Font(14.0f));
    addAndMakeVisible(usernameInput);

    //Password Label
    passwordLabel.setText("Password:", juce::dontSendNotification);
    passwordLabel.setJustificationType(juce::Justification::right);
    passwordLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(passwordLabel);

    //Password Input
    passwordInput.setPasswordCharacter('*');
    passwordInput.setTextToShowWhenEmpty("Enter password", juce::Colours::grey);
    passwordInput.setFont(juce::Font(14.0f));
    addAndMakeVisible(passwordInput);

    //Login Button
    loginButton.setButtonText("Login");
    loginButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3498db));
    loginButton.onClick = [this] { handleLogin(); };
    addAndMakeVisible(loginButton);

    //Create Account Button
    createAccountButton.setButtonText("Create New Account");
    createAccountButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2ecc71));
    createAccountButton.onClick = [this] { handleCreateAccount(); };
    addAndMakeVisible(createAccountButton);

    //Message Label
    messageLabel.setJustificationType(juce::Justification::centred);
    messageLabel.setFont(juce::Font(13.0f));
    addAndMakeVisible(messageLabel);

    setSize(450, 450);
}

//Destructor
LoginComponent::~LoginComponent()
{
}

//CUstom drawing for the login screen
void LoginComponent::paint(juce::Graphics& g)
{
    //Background gradient
    g.fillAll(juce::Colour(0xff2c3e50));

    //Decorative border
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRect(getLocalBounds().reduced(20), 2);

    //Subtle inner glow, gives us aura
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.fillRect(getLocalBounds().reduced(30));
}

//Handle resizing and layout of the login screen components
void LoginComponent::resized()
{
    auto area = getLocalBounds().reduced(40);

    //Title
    titleLabel.setBounds(area.removeFromTop(50));
    area.removeFromTop(30);

    //Username row
    auto row = area.removeFromTop(30);
    usernameLabel.setBounds(row.removeFromLeft(100));
    row.removeFromLeft(10);
    usernameInput.setBounds(row);
    area.removeFromTop(15);

    //Password row
    row = area.removeFromTop(30);
    passwordLabel.setBounds(row.removeFromLeft(100));
    row.removeFromLeft(10);
    passwordInput.setBounds(row);
    area.removeFromTop(30);

    //Buttons
    loginButton.setBounds(area.removeFromTop(40));
    area.removeFromTop(15);
    createAccountButton.setBounds(area.removeFromTop(40));
    area.removeFromTop(20);

    //Message
    messageLabel.setBounds(area.removeFromTop(30));
}

//This is the logic for login
void LoginComponent::handleLogin()
{
    //Grab the input values from the UI and remove whitespace
    juce::String username = usernameInput.getText().trim();
    juce::String password = passwordInput.getText();

    //Validate that the user is not brain dead and actually entered soemthing
    if (username.isEmpty() || password.isEmpty())
    {
        //Tell monkey brain what to do, and throw some red at it to make it scary
        setMessage("Please enter username and password", juce::Colours::red);
        return;
    }
	//Call the handle for the login, which will be implemented in the parent component,
    //and will handle the actual login logic and transition to the next screen if successful
    if (onLogin)
        onLogin(username, password);
}

//Handle the create account logic, lots of callbacks bois
void LoginComponent::handleCreateAccount()
{
    if (onCreateAccount)
        onCreateAccount();
}

//Helper method (might be using this more often? maybe?)
void LoginComponent::setMessage(const juce::String& message, juce::Colour colour)
{
    messageLabel.setText(message, juce::dontSendNotification);
    messageLabel.setColour(juce::Label::textColourId, colour);
}

//Another helper method to clear all input fields
void LoginComponent::clearInputs()
{
    usernameInput.clear();
    passwordInput.clear();
    messageLabel.setText("", juce::dontSendNotification);
}
