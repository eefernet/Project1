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
    clusterEngine(ownerDashboard.getSoundLibrary()),
    clusterPage(clusterEngine)
{
    // Setup LoginComponent callbacks
    loginScreen.onOwnerLogin = [this](juce::String username, juce::String password) {
        handleOwnerLogin(username, password);
        };
	//This is the callback for when the user clicks the "Continue as Guest" button on the login screen
    loginScreen.onGuestLogin = [this]() {
        handleGuestLogin();
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
        currentUser.reset();
        showView(ViewState::Login);
        loginScreen.setMessage("Logged out successfully", juce::Colours::green);
        };
    guestDashboard.onLogout = [this]() {
        currentUser.reset();
        showView(ViewState::Login);
        loginScreen.setMessage("Logged out successfully", juce::Colours::green);
        };

    // Show login screen initially
    addAndMakeVisible(loginScreen);
	//Account setup is added but hidden until needed
    accountSetup.setVisible(false);
	//Add account setup to the main component so we can switch to it when needed
    addChildComponent(accountSetup);
    //Dashboards are added bbut hidden until we need them
    ownerDashboard.setVisible(false);
    //add owner Dashoard as a child to the main component
    addChildComponent(ownerDashboard);
    guestDashboard.setVisible(false);
	//Add guest dashboard as a child to the main component
    addChildComponent(guestDashboard);

    clusterPage.setVisible(false);
    addChildComponent(clusterPage);

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


	//Set first size of the main window, its resizable so size doesnt matter i guess
    //need to update thee size to see the load button?
    setSize(800, 600);
}
//destructor
MainComponent::~MainComponent()
{
}

//Painting method
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

//setting bounds for the child components
void MainComponent::resized()
{
    loginScreen.setBounds(getLocalBounds());
    accountSetup.setBounds(getLocalBounds());
    ownerDashboard.setBounds(getLocalBounds());
    guestDashboard.setBounds(getLocalBounds());
    clusterPage.setBounds(getLocalBounds());
}

//Logic when the owner tries login, check if he exists, if the password is correct, and if he is an owner. If all checks pass, transition to the owner dashboard
void MainComponent::handleOwnerLogin(juce::String username, juce::String password)
{
	//Log output for debugging purposes, we can remove this later
    DBG("Attempting Owner login: " << username);

    // Find user
    User* user = findUser(username);

    //If we get a nullptr that means the user is not found
    if (user == nullptr)
    {
        loginScreen.setMessage("User not found", juce::Colours::red);
        //Log output
        DBG("User not found: " << username);
        //The rest of the code doesnt matter so return 
        return;
    }

    // Check if it's an Owner
    if (user->getUserRole() != UserRole::Owner)
    {
        //If the user is found but does not have the owner role show error and return
        loginScreen.setMessage("This account is not an Owner account", juce::Colours::red);
        //Log output
        DBG("User is not an Owner: " << username);
		//Return because we dont want to attempt login if the user is not an owner
        return;
    }

    // Attempt login
    if (user->login(password))
    {
		//Login successful, set current user and transition to owner dashboard
        currentUser.reset(dynamic_cast<Owner*>(user));
		//Show success message 
        loginScreen.setMessage("Login successful!", juce::Colours::green);
		//Log output
        DBG("Owner login successful: " << username);

        // Show welcome message
        currentUser->displayWelcome();

        // Transition to Owner dashboard (Sprint 2)
        showOwnerDashboard();
    }
	//failure to login, show error message and log output
    else
    {
        loginScreen.setMessage("Incorrect password", juce::Colours::red);
        DBG("Incorrect password for: " << username);
    }
}

//Logic when the guest tries to login
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

//Logic for handling when user clicks create account
void MainComponent::handleCreateAccountRequest()
{
	//Log output
    DBG("Switching to account creation");
	//Show account setup view, logic will bbe handled in the AccountSetupComponent and the callback to handleAccountCreated
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

    // Show success message and return to login
    //TODO: now thinking about it, i dont need that sleep call in AccountSetupComponent.cpp. I believe sleep is a blocking call
    juce::Timer::callAfterDelay(1500, [this]() {
        showView(ViewState::Login);
        loginScreen.setMessage("Account created! Please login", juce::Colours::green);
        });
}
//This is how we handle when the user cancels accounbt creation, clear everything for reuse and segue to the prev screen (login)
void MainComponent::handleCancelAccountSetup()
{
    DBG("Cancelled account setup");
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
	//for the dashboards but since they are not implemented yet, we will just show a message in the login screen when we transition to them
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

    //remove later
    // For now, just show a message
    //loginScreen.setMessage("Owner Dashboard - Coming in Sprint 2!", juce::Colours::cyan);
}

//This is a placeholder!!!
void MainComponent::showGuestDashboard()
{
    
    DBG("=== GUEST DASHBOARD ===");
    showView(ViewState::GuestDashboard);

    //remove later
    // For now, just show a message
    //loginScreen.setMessage("Guest Dashboard - Coming in Sprint 2!", juce::Colours::cyan);
}
void MainComponent::showClusterView()
{
    DBG("=== CLUSTER VIEW ===");
    showView(ViewState::ClusterView);
}
