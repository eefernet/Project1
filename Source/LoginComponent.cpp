/*
  ==============================================================================

    LoginComponent.cpp
    Created: 10 Feb 2026 6:02:49pm
    Author:  ethan

  ==============================================================================
*/

#include "LoginComponent.h"
#include "UIController.h"

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
    g.fillAll(juce::Colour(UIController::bg));
}

//Handle resizing and layout of the login screen components
void LoginComponent::resized()
{
    auto area = getLocalBounds().reduced(40);

    //Center column — cap the form width so it doesn't stretch on wide windows
    int formWidth = juce::jmin(360, area.getWidth());
    int centreX = area.getX() + (area.getWidth() - formWidth) / 2;

    //Title — full width so it stays centred
    titleLabel.setBounds(area.removeFromTop(50));
    area.removeFromTop(30);

    //Username row
    auto row = area.removeFromTop(30);
    int labelW = 90;
    int gap = 10;
    int inputW = formWidth - labelW - gap;
    int rowY = row.getY();
    usernameLabel.setBounds(centreX, rowY, labelW, 30);
    usernameInput.setBounds(centreX + labelW + gap, rowY, inputW, 30);
    area.removeFromTop(15);

    //Password row
    row = area.removeFromTop(30);
    rowY = row.getY();
    passwordLabel.setBounds(centreX, rowY, labelW, 30);
    passwordInput.setBounds(centreX + labelW + gap, rowY, inputW, 30);
    area.removeFromTop(30);

    //Buttons — fixed width, centred
    int btnWidth = 200;
    int btnX = area.getX() + (area.getWidth() - btnWidth) / 2;

    row = area.removeFromTop(40);
    loginButton.setBounds(btnX, row.getY(), btnWidth, 40);
    area.removeFromTop(15);

    row = area.removeFromTop(40);
    createAccountButton.setBounds(btnX, row.getY(), btnWidth, 40);
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
