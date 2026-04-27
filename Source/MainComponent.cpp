#include "MainComponent.h"


//This is the parent view that manages the login and account creation components, as well as user management.
// It will also handle the transitions between different views (login, account setup, owner dashboard, guest dashboard)
// based on user interactions and roles. For Sprint 1, it focuses on the login and account creation flow, while Sprint
// 2 and 3 will expand to include the dashboards and additional features.

//Constructor sets up callbacks and initializes the first view

MainComponent::MainComponent()
    : accountSetup(true), // First user
    nextUserId(1),
    currentView(ViewState::Login),
    guestDashboard(ownerDashboard.getSoundLibrary()),
    clusterEngine(ownerDashboard.getSoundLibrary()),
    clusterPage(clusterEngine, ownerDashboard.getSoundLibrary())
{
    juce::LookAndFeel::setDefaultLookAndFeel(&uiController);

    // Setup LoginComponent callbacks
    loginScreen.onLogin = [this](juce::String username, juce::String password) {
        handleLogin(username, password);
        };
	//This is the callback for when the user clicks the "Create Account" button on the login screen, it will switch to the account setup view
    loginScreen.onCreateAccount = [this]() {
        handleCreateAccountRequest();
        };

    // Setup AccountSetupComponent callbacks
    accountSetup.onAccountCreated = [this](juce::String username, juce::String password, UserRole role) {
        handleAccountCreated(username, password, role);
        };
	//This is the callback for when the user clicks the "Cancel" button on the account setup screen, it will switch back to the login view
    accountSetup.onCancel = [this]() {
        handleCancelAccountSetup();
        };

    //Setup place holder dashboards for logout callbacks
    ownerDashboard.onLogout = [this]() {
        currentUser = nullptr;
        showView(ViewState::Login);
        loginScreen.setMessage("Logged out successfully", juce::Colour(UIController::successText));
        };
    guestDashboard.onLogout = [this]() {
        currentUser = nullptr;
        showView(ViewState::Login);
        loginScreen.setMessage("Logged out successfully", juce::Colour(UIController::successText));
        };

    clusterPage.back = [this]()
        {
            showView(lastDashboardView);
        };

    ownerDashboard.viewCluster = [this]()
        {
            lastDashboardView = ViewState::OwnerDashboard;
            showView(ViewState::ClusterView);
        };

    guestDashboard.viewCluster = [this]()
        {
            lastDashboardView = ViewState::GuestDashboard;
            showView(ViewState::ClusterView);
        };

    // Audio Workstation setup
    audioWorkstation.onBack = [this]()
        {
            showView(lastDashboardView);
        };

    ownerDashboard.viewRecorder = [this]()
        {
            lastDashboardView = ViewState::OwnerDashboard;
            audioWorkstation.setSoundsFolder(ownerDashboard.getSoundsFolder());
            showView(ViewState::RecorderView);
        };

    audioWorkstation.onRecordingSaved = [this](const juce::File& f)
        {
            ownerDashboard.addRecording(f);
        };

    ownerDashboard.createGuestAccount = [this]()
        {
            handleCreateGuestAccountRequest();
        };

    // Show login screen initially
    addAndMakeVisible(loginScreen);
	//Account setup is added but hidden until needed
    accountSetup.setVisible(false);
	//Add account setup to the main component so we can switch to it when needed
    addChildComponent(accountSetup);
    //Dashboards are added but hidden until we need them
    ownerDashboard.setVisible(false);
    //add owner Dashboard as a child to the main component
    addChildComponent(ownerDashboard);
    guestDashboard.setVisible(false);
	//Add guest dashboard as a child to the main component
    addChildComponent(guestDashboard);

    clusterPage.setVisible(false);
    addChildComponent(clusterPage);

    audioWorkstation.setVisible(false);
    addChildComponent(audioWorkstation);

	//Set first size of the main window, its resizable so size doesnt matter i guess
    setSize(1280, 720);
}
//destructor
MainComponent::~MainComponent()
{
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

//Painting method
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(UIController::bg));
}

//setting bounds for the child components
void MainComponent::resized()
{
    loginScreen.setBounds(getLocalBounds());
    accountSetup.setBounds(getLocalBounds());
    ownerDashboard.setBounds(getLocalBounds());
    guestDashboard.setBounds(getLocalBounds());
    clusterPage.setBounds(getLocalBounds());
    audioWorkstation.setBounds(getLocalBounds());
}

//Unified login handler — checks credentials for both owners and guests, routes to correct dashboard
void MainComponent::handleLogin(juce::String username, juce::String password)
{
	//Log output for debugging purposes, we can remove this later
    DBG("Attempting login: " << username);

    // Find user
    User* user = findUser(username);

    //If we get a nullptr that means the user is not found
    if (user == nullptr)
    {
        loginScreen.setMessage("User not found", juce::Colour(UIController::errorText));
        DBG("User not found: " << username);
        return;
    }

    // Attempt login
    if (user->login(password))
    {
		//Login successful — point currentUser at the entry in allUsers (non-owning)
        currentUser = user;
		//Show success message
        loginScreen.setMessage("Login successful!", juce::Colour(UIController::successText));
        DBG("Login successful: " << username << " as " << (user->getUserRole() == UserRole::Owner ? "Owner" : "Guest"));

        // Show welcome message
        currentUser->displayWelcome();

        // Route to the correct dashboard based on role
        if (currentUser->getUserRole() == UserRole::Owner)
            showOwnerDashboard();
        else
            showGuestDashboard();
    }
	//failure to login, show error message and log output
    else
    {
        loginScreen.setMessage("Incorrect password", juce::Colour(UIController::errorText));
        DBG("Incorrect password for: " << username);
    }
}

//Logic for handling when user clicks create account from login screen
void MainComponent::handleCreateAccountRequest()
{
	//Log output
    DBG("Switching to account creation");
    //Update first user flag and configure for owner creation from login screen
    accountSetup.setFirstUser(isFirstUser());
    accountSetup.setGuestOnly(false);
	//Show account setup view, logic will be handled in the AccountSetupComponent and the callback to handleAccountCreated
    showView(ViewState::AccountSetup);
}

//Logic for handling when owner clicks create guest account from dashboard
void MainComponent::handleCreateGuestAccountRequest()
{
    DBG("Switching to guest account creation from owner dashboard");
    accountSetup.setFirstUser(false);
    accountSetup.setGuestOnly(true);
    showView(ViewState::AccountSetup);
}

//Logic for new account creation
void MainComponent::handleAccountCreated(juce::String username, juce::String password, UserRole role)
{
    //Log output
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

    //Check user role if owner or guest, give a unique id and add to list
    if (role == UserRole::Owner)
    {
        newUser = std::make_unique<Owner>(nextUserId++, username, password);
        DBG("Created Owner account: " << username);
    }
	//If its not an owner, then it must be a guest, so we create a guest account
    else
    {
        newUser = std::make_unique<Guest>(nextUserId++, username, password);
        DBG("Created Guest account: " << username);
    }
	//Add the new user to the list of all users, transfer ownership of the new user to the vector using std::move since unique_ptr cannot be copied
    allUsers.push_back(std::move(newUser));

    DBG("Account created successfully. Total users: " << allUsers.size());

    // If owner is creating a guest account, return to owner dashboard; otherwise go to login
    if (currentUser != nullptr && currentUser->getUserRole() == UserRole::Owner)
    {
        showView(ViewState::OwnerDashboard);
    }
    else
    {
        showView(ViewState::Login);
        loginScreen.setMessage("Account created! Please login", juce::Colour(UIController::successText));
    }
}
//This is how we handle when the user cancels account creation, return to wherever they came from
void MainComponent::handleCancelAccountSetup()
{
    DBG("Cancelled account setup");
    //If the current user is logged in (owner creating guest), go back to owner dashboard
    if (currentUser != nullptr && currentUser->getUserRole() == UserRole::Owner)
        showView(ViewState::OwnerDashboard);
    else
        showView(ViewState::Login);
}
//Helper to switch between views, we can expand this as we add more windows
void MainComponent::showView(ViewState view)
{
    currentView = view;
	//Show/hide components based on the current view
    loginScreen.setVisible(view == ViewState::Login);
    accountSetup.setVisible(view == ViewState::AccountSetup);
    ownerDashboard.setVisible(view == ViewState::OwnerDashboard);
    guestDashboard.setVisible(view == ViewState::GuestDashboard);
    clusterPage.setVisible(view == ViewState::ClusterView);
    audioWorkstation.setVisible(view == ViewState::RecorderView);

    if (view == ViewState::Login)
    {
        loginScreen.clearInputs();
    }
}

//Helper to find user by username, returns nullptr if not found
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
//This is a placeholder!!!
void MainComponent::showOwnerDashboard()
{
    DBG("=== OWNER DASHBOARD ===");
    ownerDashboard.setUsername(currentUser->getUserName());
    showView(ViewState::OwnerDashboard);
}

//This is a placeholder!!!
void MainComponent::showGuestDashboard()
{
    DBG("=== GUEST DASHBOARD ===");
    guestDashboard.setUsername(currentUser->getUserName());
    showView(ViewState::GuestDashboard);
}
void MainComponent::showClusterView()
{
    DBG("=== CLUSTER VIEW ===");
    showView(ViewState::ClusterView);
}
void MainComponent::showRecorderView()
{
    DBG("=== RECORDER VIEW ===");
    showView(ViewState::RecorderView);
}
