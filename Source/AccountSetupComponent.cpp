/*
  ==============================================================================

    AccountSetupComponent.cpp
    Created: 10 Feb 2026 6:03:10pm
    Author:  ethan

  ==============================================================================
*/

#include "AccountSetupComponent.h"
#include "UIController.h"

// Check if this is the first user being created, if so, they must be an owner
AccountSetupComponent::AccountSetupComponent(bool firstUser): isFirstUser(firstUser), guestOnly(false)
{
    //Title
    titleLabel.setText("Create New Account", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::titleText));
    addAndMakeVisible(titleLabel);

    //Username
    usernameLabel.setText("Username:", juce::dontSendNotification);
    usernameLabel.setJustificationType(juce::Justification::right);
    usernameLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::titleText));
    addAndMakeVisible(usernameLabel);

    usernameInput.setTextToShowWhenEmpty("Enter username", juce::Colour(UIController::placeholder));
    addAndMakeVisible(usernameInput);

    //Password
    passwordLabel.setText("Password:", juce::dontSendNotification);
    passwordLabel.setJustificationType(juce::Justification::right);
    passwordLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::titleText));
    addAndMakeVisible(passwordLabel);

    passwordInput.setPasswordCharacter('*');
    passwordInput.setTextToShowWhenEmpty("Enter password", juce::Colour(UIController::placeholder));
    addAndMakeVisible(passwordInput);

    //Confirm Password
    confirmPasswordLabel.setText("Confirm Password:", juce::dontSendNotification);
    confirmPasswordLabel.setJustificationType(juce::Justification::right);
    confirmPasswordLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::titleText));
    addAndMakeVisible(confirmPasswordLabel);

    confirmPasswordInput.setPasswordCharacter('*');
    confirmPasswordInput.setTextToShowWhenEmpty("Re-enter password", juce::Colour(UIController::placeholder));
    addAndMakeVisible(confirmPasswordInput);

    //Role Selector
    roleLabel.setText("Account Type:", juce::dontSendNotification);
    roleLabel.setJustificationType(juce::Justification::right);
    roleLabel.setVisible(false);  // Hidden — role is determined automatically
    addAndMakeVisible(roleLabel);

    roleSelector.addItem("Owner", 1);
    roleSelector.addItem("Guest", 2);
    roleSelector.setSelectedId(isFirstUser ? 1 : 2);  // First user must be Owner
    roleSelector.setEnabled(!isFirstUser);  // Disable if first user
    roleSelector.setVisible(false);  // Hidden — role is determined automatically
    addAndMakeVisible(roleSelector);

	//Buttons tied to their individual callbacks
    createAccountButton.setButtonText("Create Account");
    createAccountButton.onClick = [this] { onCreateAccount(); };
    addAndMakeVisible(createAccountButton);

    //Cancel button and its callback
    cancelButton.setButtonText("Cancel");
    cancelButton.onClick = [this] { onCancelClicked(); };
    addAndMakeVisible(cancelButton);

    //Message Label
    messageLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(messageLabel);

	//If the first user is being created, show a message that they must be an owner
    if (isFirstUser)
    {
        messageLabel.setText("First user must be an Owner", juce::dontSendNotification);
        messageLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::warnText));
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
    g.fillAll(juce::Colour(UIController::bg));
}

//Setting bounds for UI elements, onyl called when the window is resized
void AccountSetupComponent::resized()
{
    auto area = getLocalBounds().reduced(40);

    // Center the form column — cap width so fields don't stretch across the window
    const int formWidth = juce::jmin(380, area.getWidth());
    const int centreX   = area.getX() + (area.getWidth() - formWidth) / 2;
    const int labelW    = 130;
    const int gap       = 10;
    const int inputW    = formWidth - labelW - gap;
    const int rowH      = 30;
    const int rowGap    = 12;

    // Title — full width centred
    titleLabel.setBounds(area.removeFromTop(50));
    area.removeFromTop(24);

    auto placeRow = [&](juce::Label& lbl, juce::Component& field)
    {
        auto row = area.removeFromTop(rowH);
        int y = row.getY();
        lbl.setBounds   (centreX,                   y, labelW, rowH);
        field.setBounds (centreX + labelW + gap,    y, inputW, rowH);
        area.removeFromTop(rowGap);
    };

    placeRow(usernameLabel,        usernameInput);
    placeRow(passwordLabel,        passwordInput);
    placeRow(confirmPasswordLabel, confirmPasswordInput);

    // Role row (hidden but still laid out for safety)
    if (roleLabel.isVisible() || roleSelector.isVisible())
        placeRow(roleLabel, roleSelector);

    area.removeFromTop(18);

    // Buttons — fixed width, centered as a pair
    const int btnW = 170;
    const int btnH = 38;
    const int btnGap = 20;
    const int btnTotal = btnW * 2 + btnGap;
    const int btnX = area.getX() + (area.getWidth() - btnTotal) / 2;

    auto buttonRow = area.removeFromTop(btnH);
    createAccountButton.setBounds(btnX,                        buttonRow.getY(), btnW, btnH);
    cancelButton.setBounds       (btnX + btnW + btnGap,        buttonRow.getY(), btnW, btnH);
    area.removeFromTop(18);

    // Message — centered under buttons
    messageLabel.setBounds(centreX, area.getY(), formWidth, rowH);
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
        messageLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::errorText));
        return;
    }
    //Password cannot be empty
    if (password.isEmpty())
    {
        messageLabel.setText("Password cannot be empty", juce::dontSendNotification);
        messageLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::errorText));
        return;
    }
    //Passwords must match in each field
    if (password != confirmPassword)
    {
        messageLabel.setText("Passwords do not match", juce::dontSendNotification);
        messageLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::errorText));
        return;
    }

	//Determine the role: first user is always owner, guestOnly mode forces guest, otherwise use selector
    UserRole selectedRole;
    if (isFirstUser)
        selectedRole = UserRole::Owner;
    else if (guestOnly)
        selectedRole = UserRole::Guest;
    else
        selectedRole = (roleSelector.getSelectedId() == 1) ? UserRole::Owner : UserRole::Guest;

	//Call the callback to create the account in parent component
    if (onAccountCreated)
    {
        onAccountCreated(username, password, selectedRole);
    }

	//Show success message to make the user feel good about themselves, that for once they did something right in life
    //Not sure why this is not displaying
    messageLabel.setText("Account created successfully!", juce::dontSendNotification);
    messageLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::successText));

    // Sleep for a second to let the user see the success message
	//_sleep(1000); 
    //Reset fields for next usage
	ResetFields();
    
}

//THis is how we handle when the user cancels accounbt creation, clear everything for reuse and segue to the prev screen (login)
void AccountSetupComponent::onCancelClicked()
{
    //Clear all fields
	ResetFields();

    //Let parent know the child is done from daycare
    // TODO: remove later? Was borked but im sure thats because of my imp of the constructor and the onCancel function
    //getParentComponent()->setVisible(true);
    if(onCancel)
    {
        onCancel();
	}

}

//This is a helper function to reset the input fields
//TODO: This is kinda broken, but i want to drink alcohol so ill fix later
void AccountSetupComponent::ResetFields()
{
    usernameInput.clear();
    passwordInput.clear();
    confirmPasswordInput.clear();
    messageLabel.setText("", juce::dontSendNotification);
}

//Configure the component for guest-only account creation (called from owner dashboard)
void AccountSetupComponent::setGuestOnly(bool guestOnlyMode)
{
    guestOnly = guestOnlyMode;
    if (guestOnly)
    {
        titleLabel.setText("Create Guest Account", juce::dontSendNotification);
        messageLabel.setText("Creating a guest account", juce::dontSendNotification);
        messageLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::warnText));
        roleSelector.setSelectedId(2);  // Guest
    }
    else
    {
        titleLabel.setText("Create New Account", juce::dontSendNotification);
        if (isFirstUser)
        {
            messageLabel.setText("First user must be an Owner", juce::dontSendNotification);
            messageLabel.setColour(juce::Label::textColourId, juce::Colour(UIController::warnText));
            roleSelector.setSelectedId(1);  // Owner
        }
        else
        {
            messageLabel.setText("", juce::dontSendNotification);
        }
    }
}

//Update the first user flag dynamically
void AccountSetupComponent::setFirstUser(bool first)
{
    isFirstUser = first;
}

