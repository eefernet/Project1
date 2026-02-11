#include "MainComponent.h"



MainComponent::MainComponent()
    : accountSetup(true), // First user
    nextUserId(1),
    currentView(ViewState::Login)
{
    // Setup LoginComponent callbacks
    loginScreen.onOwnerLogin = [this](juce::String username, juce::String password) {
        handleOwnerLogin(username, password);
        };

    loginScreen.onGuestLogin = [this]() {
        handleGuestLogin();
        };

    loginScreen.onCreateAccount = [this]() {
        handleCreateAccountRequest();
        };

    // Setup AccountSetupComponent callbacks
    accountSetup.onAccountCreated = [this](juce::String username, juce::String password, UserRole role) {
        handleAccountCreated(username, password, role);
        };

    accountSetup.onCancel = [this]() {
        handleCancelAccountSetup();
        };

    // Show login screen initially
    addAndMakeVisible(loginScreen);
    accountSetup.setVisible(false);
    addChildComponent(accountSetup);

    setSize(600, 500);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void MainComponent::resized()
{
    loginScreen.setBounds(getLocalBounds());
    accountSetup.setBounds(getLocalBounds());
}

void MainComponent::handleOwnerLogin(juce::String username, juce::String password)
{
    DBG("Attempting Owner login: " << username);

    // Find user
    User* user = findUser(username);

    if (user == nullptr)
    {
        loginScreen.setMessage("User not found", juce::Colours::red);
        DBG("User not found: " << username);
        return;
    }

    // Check if it's an Owner
    if (user->getUserRole() != UserRole::Owner)
    {
        loginScreen.setMessage("This account is not an Owner account", juce::Colours::red);
        DBG("User is not an Owner: " << username);
        return;
    }

    // Attempt login
    if (user->login(password))
    {
        currentUser.reset(dynamic_cast<Owner*>(user));
        loginScreen.setMessage("Login successful!", juce::Colours::green);
        DBG("Owner login successful: " << username);

        // Show welcome message
        currentUser->displayWelcome();

        // Transition to Owner dashboard (Sprint 2)
        showOwnerDashboard();
    }
    else
    {
        loginScreen.setMessage("Incorrect password", juce::Colours::red);
        DBG("Incorrect password for: " << username);
    }
}

void MainComponent::handleGuestLogin()
{
    DBG("Guest login");

    // Create temporary guest user
    currentUser = std::make_unique<Guest>(0, "Guest", "");
    currentUser->displayWelcome();

    loginScreen.setMessage("Logged in as Guest", juce::Colours::green);

    // Transition to Guest dashboard (Sprint 2)
    showGuestDashboard();
}

void MainComponent::handleCreateAccountRequest()
{
    DBG("Switching to account creation");
    showView(ViewState::AccountSetup);
}

void MainComponent::handleAccountCreated(juce::String username, juce::String password, UserRole role)
{
    DBG("Creating new account: " << username << " as " << (role == UserRole::Owner ? "Owner" : "Guest"));

    // Check if username already exists
    if (findUser(username) != nullptr)
    {
        DBG("Username already exists: " << username);
        // AccountSetupComponent will show error message
        return;
    }

    // Create the new user
    std::unique_ptr<User> newUser;

    if (role == UserRole::Owner)
    {
        newUser = std::make_unique<Owner>(nextUserId++, username, password);
        DBG("Created Owner account: " << username);
    }
    else
    {
        newUser = std::make_unique<Guest>(nextUserId++, username, password);
        DBG("Created Guest account: " << username);
    }

    allUsers.push_back(std::move(newUser));

    DBG("Account created successfully. Total users: " << allUsers.size());

    // Show success message and return to login
    juce::Timer::callAfterDelay(1500, [this]() {
        showView(ViewState::Login);
        loginScreen.setMessage("Account created! Please login", juce::Colours::green);
        });
}

void MainComponent::handleCancelAccountSetup()
{
    DBG("Cancelled account setup");
    showView(ViewState::Login);
}

void MainComponent::showView(ViewState view)
{
    currentView = view;

    loginScreen.setVisible(view == ViewState::Login);
    accountSetup.setVisible(view == ViewState::AccountSetup);

    if (view == ViewState::Login)
    {
        loginScreen.clearInputs();
    }
}

User* MainComponent::findUser(const juce::String& username)
{
    for (auto& user : allUsers)
    {
        if (user->getUserName() == username)
        {
            return user.get();
        }
    }
    return nullptr;
}

bool MainComponent::isFirstUser() const
{
    return allUsers.empty();
}

void MainComponent::showOwnerDashboard()
{
    DBG("=== OWNER DASHBOARD ===");
    DBG("TODO: Implement Owner dashboard in Sprint 2");
    DBG("Features: Record, View Waveform, Play, Filter, Save, etc.");

    // For now, just show a message
    loginScreen.setMessage("Owner Dashboard - Coming in Sprint 2!", juce::Colours::cyan);
}

void MainComponent::showGuestDashboard()
{
    DBG("=== GUEST DASHBOARD ===");
    DBG("TODO: Implement Guest dashboard in Sprint 2");
    DBG("Features: View Sound List, Play & Filter, Download");

    // For now, just show a message
    loginScreen.setMessage("Guest Dashboard - Coming in Sprint 2!", juce::Colours::cyan);
}