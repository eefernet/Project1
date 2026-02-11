/*
  ==============================================================================

    AccountSetupComponent.cpp
    Created: 10 Feb 2026 6:03:10pm
    Author:  ethan

  ==============================================================================
*/

#include "AccountSetupComponent.h"

// Check if this is the first user being created, if so, they must be an owner
AccountSetupComponent::AccountSetupComponent(): isFirstUser(true), isOwner(false)  
{
    //Title
    titleLabel.setText("Create New Account", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    //Username
    usernameLabel.setText("Username:", juce::dontSendNotification);
    usernameLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(usernameLabel);

    usernameInput.setTextToShowWhenEmpty("Enter username", juce::Colours::grey);
    addAndMakeVisible(usernameInput);

    //Password
    passwordLabel.setText("Password:", juce::dontSendNotification);
    passwordLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(passwordLabel);

    passwordInput.setPasswordCharacter('*');
    passwordInput.setTextToShowWhenEmpty("Enter password", juce::Colours::grey);
    addAndMakeVisible(passwordInput);

    //Confirm Password
    confirmPasswordLabel.setText("Confirm Password:", juce::dontSendNotification);
    confirmPasswordLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(confirmPasswordLabel);

    confirmPasswordInput.setPasswordCharacter('*');
    confirmPasswordInput.setTextToShowWhenEmpty("Re-enter password", juce::Colours::grey);
    addAndMakeVisible(confirmPasswordInput);

    //Role Selector
    roleLabel.setText("Account Type:", juce::dontSendNotification);
    roleLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(roleLabel);

    roleSelector.addItem("Owner", 1);
    roleSelector.addItem("Guest", 2);
    roleSelector.setSelectedId(isFirstUser ? 1 : 2);  // First user must be Owner
    roleSelector.setEnabled(!isFirstUser);  // Disable if first user
    addAndMakeVisible(roleSelector);

    //Buttons
    createAccountButton.setButtonText("Create Account");
    createAccountButton.onClick = [this] { onCreateAccount(); };
    addAndMakeVisible(createAccountButton);

    cancelButton.setButtonText("Cancel");
    cancelButton.onClick = [this] { onCancel(); };
    addAndMakeVisible(cancelButton);

    //Message Label
    messageLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(messageLabel);

	//If the first user is being created, show a message that they must be an owner
    if (isFirstUser)
    {
        messageLabel.setText("First user must be an Owner", juce::dontSendNotification);
        messageLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    }
    //Size matters folks
    setSize(450, 450);
}

//Destructor (not really doign anything atm)
AccountSetupComponent::~AccountSetupComponent()
{
}

//This is where all the custom drawing happens
void AccountSetupComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds().reduced(20), 2);
}

//Setting bounds for UI elements, onyl called when the window is resized
void AccountSetupComponent::resized()
{
    auto area = getLocalBounds().reduced(40);

    titleLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(20);

    //Username (we have to do this for every element)
    auto row = area.removeFromTop(30);
    usernameLabel.setBounds(row.removeFromLeft(150));
    row.removeFromLeft(10);
    usernameInput.setBounds(row);
    area.removeFromTop(15);

    //Password
    row = area.removeFromTop(30);
    passwordLabel.setBounds(row.removeFromLeft(150));
    row.removeFromLeft(10);
    passwordInput.setBounds(row);
    area.removeFromTop(15);

    //Confirm Password
    row = area.removeFromTop(30);
    confirmPasswordLabel.setBounds(row.removeFromLeft(150));
    row.removeFromLeft(10);
    confirmPasswordInput.setBounds(row);
    area.removeFromTop(15);

    //Role
    row = area.removeFromTop(30);
    roleLabel.setBounds(row.removeFromLeft(150));
    row.removeFromLeft(10);
    roleSelector.setBounds(row);
    area.removeFromTop(30);

    //Buttons
    auto buttonRow = area.removeFromTop(35);
    createAccountButton.setBounds(buttonRow.removeFromLeft(150));
    buttonRow.removeFromLeft(20);
    cancelButton.setBounds(buttonRow.removeFromLeft(150));
    area.removeFromTop(15);

    //Message
    messageLabel.setBounds(area.removeFromTop(30));
}

//This is where we handle the logic for creating an account, including validation and calling the callback to create the account in the parent component
void AccountSetupComponent::onCreateAccount()
{
    //Grab the input values from the UI
    juce::String username = usernameInput.getText();
    juce::String password = passwordInput.getText();
    juce::String confirmPassword = confirmPasswordInput.getText();

    //Now lets validate that shit
    //Make sure the username is not empty
    if (username.isEmpty())
    {
        messageLabel.setText("Username cannot be empty", juce::dontSendNotification);
        messageLabel.setColour(juce::Label::textColourId, juce::Colours::red);
        return;
    }
    //Password cannot be empty
    if (password.isEmpty())
    {
        messageLabel.setText("Password cannot be empty", juce::dontSendNotification);
        messageLabel.setColour(juce::Label::textColourId, juce::Colours::red);
        return;
    }
    //Passwords must match in each field
    if (password != confirmPassword)
    {
        messageLabel.setText("Passwords do not match", juce::dontSendNotification);
        messageLabel.setColour(juce::Label::textColourId, juce::Colours::red);
        return;
    }

	//Get the selected role, if this is the first user, they must be an owner, otherwise we can get the role from the selector
    UserRole selectedRole = (roleSelector.getSelectedId() == 1) ? UserRole::Owner : UserRole::Guest;

	//Call the callback to create the account in parent component
    if (onAccountCreated)
    {
        onAccountCreated(username, password, selectedRole);
    }

	//Show success message to make the user feel good about themselves, that for once they did something right in life
    messageLabel.setText("Account created successfully!", juce::dontSendNotification);
    messageLabel.setColour(juce::Label::textColourId, juce::Colours::green);
}

//THis is how we handle when the user cancels accounbt creation, clear everything for reuse and segue to the prev screen (login)
void AccountSetupComponent::onCancel()
{
    //Clear all fields
    usernameInput.clear();
    passwordInput.clear();
    confirmPasswordInput.clear();
    messageLabel.setText("", juce::dontSendNotification);

    //Let parent know the child is done from daycare
    getParentComponent()->setVisible(true);
}