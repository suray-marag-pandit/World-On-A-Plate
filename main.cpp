#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <algorithm>
#include <ctime>

using namespace std;
using namespace rapidjson;

// Struct for MenuItem
struct MenuItem
{
    int serialNumber;
    string category;
    string description;
    double price;
    int demandCount = 0; // To track popularity
};

// Struct for Order
struct Order
{
    int chefId;
    vector<MenuItem> items;
    double totalCost = 0;
};

// Struct for Reservation
struct Reservation
{
    string name;
    vector<int> tableNumbers; // Allows multiple tables per reservation
};

class Menu
{
    vector<MenuItem> items;

public:
    void loadMenu(const string &filename);
    void showMenu(const string &category = "") const; // Updated to filter by category
    void addItem(const MenuItem &item);
    void removeItem(int serialNumber);
    void modifyItem(int serialNumber, const MenuItem &item);
    MenuItem *getItem(int serialNumber);
    const vector<MenuItem> &getItems() const { return items; }
};

struct SaleData
{
    unordered_map<int, int> timeCount;       // Maps time slots (e.g., hours) to sale counts
    unordered_map<string, int> weekdayCount; // Maps weekdays to sale counts
};

class Inventory
{
    unordered_map<string, int> stock; // Material name and quantity
public:
    void loadInventory(const string &filename);
    void showInventory();
    bool checkAvailability(const unordered_map<string, int> &requiredMaterials);
    void updateInventory(const string &material, int quantity);
};

class Admin
{
    string password = "admin123";
    Inventory inventory; // Inventory is a member of Admin
public:
    bool login();

    // Methods to manage inventory
    void manageInventory();
    void viewInventory();   // New method to view inventory
    void modifyInventory(); // New method to modify inventory

    // Accessor methods for inventory
    Inventory &getInventory() { return inventory; } // Provide access to inventory

    void manageMenu(Menu &menu);
    // void viewStatistics(const Menu& menu);
    void viewStatistics(const Menu &menu, const unordered_map<int, SaleData> &salesStatistics);
    void viewReservations(const vector<Reservation> &reservations);
};

class User
{
public:
    void viewMenu(const Menu &menu);
    Order placeOrder(Menu &menu, Inventory &inventory, int &chefCounter); // Inventory passed as a parameter
    void makeReservation(vector<Reservation> &reservations, unordered_set<int> &tableAvailability);
};

// Function to cancel a reservation
void cancelReservation(vector<Reservation> &reservations, unordered_set<int> &tableAvailability)
{
    string name;
    cout << "Enter your name to cancel reservation: ";
    cin >> name;

    auto it = find_if(reservations.begin(), reservations.end(), [&name](const Reservation &res)
                      { return res.name == name; });

    if (it != reservations.end())
    {
        for (int table : it->tableNumbers)
        {
            tableAvailability.erase(table); // Free up reserved tables
        }
        reservations.erase(it); // Remove reservation from list
        cout << "Reservation cancelled successfully.\n";
    }
    else
    {
        cout << "No reservation found under that name.\n";
    }
}
class Restaurant
{
private:
    string name;
    Menu menu;
    vector<Reservation> reservations;
    Admin admin;
    vector<Order> orders;
    unordered_set<int> tableAvailability;

    unordered_map<int, SaleData> salesStatistics; // Track statistics here
    int chefCounter = 0;

public:
    Restaurant(const string &restaurantName) : name(restaurantName) {}

    void loadMenu(const string &filename);
    void loadInventory(const string &filename);
    void loadReservationsFromFile();
    void saveReservationsToFile();
    void loadStatisticsFromFile(const string &filename);
    void saveStatisticsToFile(const string &filename);

    void userInterface();
    void adminInterface();
};

// Method Implementations

void Inventory::loadInventory(const string &filename)
{
    FILE *fp = fopen(filename.c_str(), "r");
    if (!fp)
    {
        cerr << "Failed to open inventory file.\n";
        return;
    }

    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document doc;
    doc.ParseStream(is);

    fclose(fp);

    for (auto &item : doc.GetObject())
    {
        stock[item.name.GetString()] = item.value.GetInt();
    }
}

bool Inventory::checkAvailability(const unordered_map<string, int> &requiredMaterials)
{
    for (const auto &material : requiredMaterials)
    {
        if (stock[material.first] < material.second)
        {
            return false; // Not enough material available
        }
    }

    return true; // All materials available
}

void Inventory::showInventory()
{
    cout << "Inventory Levels:\n";
    for (const auto &item : stock)
    {
        cout << item.first << ": " << item.second << "\n";
    }
}

void Inventory::updateInventory(const string &material, int quantity)
{
    stock[material] = quantity;
}

void Menu::loadMenu(const string &filename)
{
    FILE *fp = fopen(filename.c_str(), "r");
    if (!fp)
    {
        cerr << "Failed to open menu file.\n";
        return;
    }

    
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document doc;
    doc.ParseStream(is);

    fclose(fp);

    for (auto &item : doc.GetArray())
    {
        MenuItem menuItem;
        menuItem.serialNumber = item["serialNumber"].GetInt();
        menuItem.category = item["category"].GetString();
        menuItem.description = item["description"].GetString();
        menuItem.price = item["price"].GetDouble();
        items.push_back(menuItem);
    }
}

void Menu::showMenu(const string &category) const
{
    cout << "Menu:\n";

    if (category.empty())
    {
        for (const auto &item : items)
        {
            cout << item.serialNumber << " - " << item.category << " - " << item.description << " - Rs. " << item.price << "\n";
        }
        return;
    }

    bool categoryFound = false;

    for (const auto &item : items)
    {
        if (item.category == category || category == "All")
        { // Show all items if category is "All"
            cout << item.serialNumber << " - " << item.category << " - " << item.description << " - Rs. " << item.price << "\n";
            categoryFound = true;
        }
    }

    if (!categoryFound)
    {
        cout << "No items found for the category: " << category << "\n";
    }
}

void Menu::addItem(const MenuItem &item)
{
    items.push_back(item);
}

void Menu::removeItem(int serialNumber)
{
    items.erase(remove_if(items.begin(), items.end(), [serialNumber](const MenuItem &item)
                          { return item.serialNumber == serialNumber; }),
                items.end());
}

void Menu::modifyItem(int serialNumber, const MenuItem &updatedItem)
{
    for (auto &item : items)
    {
        if (item.serialNumber == serialNumber)
        {
            item = updatedItem;
            return;
        }
    }
}

MenuItem *Menu::getItem(int serialNumber)
{
    for (auto &item : items)
    {
        if (item.serialNumber == serialNumber)
            return &item;
    }

    return nullptr;
}

bool Admin::login()
{
    string inputPassword;
    cout << "Enter admin password: ";
    cin >> inputPassword;

    return inputPassword == password;
}

void Admin::viewInventory()
{
    inventory.showInventory(); // Call the showInventory method from Inventory class
}

void Admin::modifyInventory()
{
    string material;
    int quantity;

    cout << "Enter material name to modify: ";
    cin >> material;

    cout << "Enter new quantity for " << material << ": ";
    cin >> quantity;

    inventory.updateInventory(material, quantity); // Update the inventory with new quantity
}

void Admin::manageInventory()
{
    int choice;

    do
    {
        cout << "1. View Inventory\n2. Modify Inventory\n0. Exit\nEnter choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            viewInventory(); // View current inventory
            break;
        case 2:
            modifyInventory(); // Modify an item in the inventory
            break;
        case 0:
            cout << "Exiting Inventory Management.\n";
            break;
        default:
            cout << "Invalid choice! Please try again.\n";
        }
    } while (choice != 0);
}

void Admin::manageMenu(Menu &menu)
{
    int choice;

    cout << "1. Add Item\n2. Remove Item\n3. Modify Item\nEnter choice: ";
    cin >> choice;

    if (choice == 1)
    {
        MenuItem item;

        cout << "Enter serial number, category, description, and price: ";
        cin >> item.serialNumber >> item.category >> item.description >> item.price;

        menu.addItem(item);
    }
    else if (choice == 2)
    {
        int serialNumber;

        cout << "Enter serial number to remove: ";
        cin >> serialNumber;

        menu.removeItem(serialNumber);
    }
    else if (choice == 3)
    {
        int serialNumber;

        cout << "Enter serial number to modify: ";
        cin >> serialNumber;

        MenuItem item;

        cout << "Enter updated category, description, and price: ";
        cin >> item.category >> item.description >> item.price;

        menu.modifyItem(serialNumber, item);
    }
}
void Restaurant::saveStatisticsToFile(const string &filename)
{
    ofstream file(filename);
    if (!file)
    {
        cerr << "Error saving statistics to file.\n";
        return;
    }

    for (const auto &[dishId, data] : salesStatistics)
    {
        file << dishId << ":\n";
        file << "Times:";
        for (const auto &[hour, count] : data.timeCount)
        {
            file << " " << hour << ":" << count;
        }
        file << "\nWeekdays:";
        for (const auto &[day, count] : data.weekdayCount)
        {
            file << " " << day << ":" << count;
        }
        file << "\n";
    }
}

void Restaurant::loadStatisticsFromFile(const string &filename)
{
    ifstream file(filename);
    if (!file)
    {
        cerr << "Error loading statistics from file.\n";
        return;
    }

    int dishId;
    string line;
    while (getline(file, line))
    {
        if (line.back() == ':')
        {
            dishId = stoi(line.substr(0, line.size() - 1));
        }
        else if (line.find("Times:") == 0)
        {
            istringstream iss(line.substr(6));
            int hour, count;
            char colon;
            while (iss >> hour >> colon >> count)
            {
                salesStatistics[dishId].timeCount[hour] = count;
            }
        }
        else if (line.find("Weekdays:") == 0)
        {
            istringstream iss(line.substr(9));
            string day;
            int count;
            char colon;
            while (iss >> day >> colon >> count)
            {
                salesStatistics[dishId].weekdayCount[day] = count;
            }
        }
    }
}

// void Admin::viewStatistics(const Menu &menu)
// {
//     cout << "Statistics:\n";

//     for (const auto &item : menu.getItems())
//     {
//         cout << item.description << " sold " << item.demandCount << " times.\n";
//     }
// }

void Admin::viewReservations(const vector<Reservation> &reservations)
{
    cout << "Reservations:\n";

    for (const auto &res : reservations)
    {
        cout << res.name << " reserved tables: ";

        for (int table : res.tableNumbers)
        {
            cout << table << " ";
        }

        cout << "\n";
    }
}

void Admin::viewStatistics(const Menu &menu, const unordered_map<int, SaleData> &salesStatistics)
{
    int choice;
    cout << "Statistics Menu:\n";
    cout << "1. View Statistics by Weekday\n";
    cout << "2. View Statistics by Time\n";
    cout << "0. Exit\n";
    cout << "Enter your choice: ";
    cin >> choice;

    switch (choice)
    {
    case 1:
    {
        string weekday;
        cout << "Enter the weekday (e.g., Monday): ";
        cin >> weekday;

        cout << "\nStatistics for " << weekday << ":\n";
        for (const auto &[dishId, data] : salesStatistics)
        {
            if (data.weekdayCount.find(weekday) != data.weekdayCount.end())
            {
                cout << "Dish " << dishId << " sold " << data.weekdayCount.at(weekday) << " times\n";
            }
        }
        break;
    }
    case 2:
    {
        cout << "\nStatistics by Time (Hour):\n";
        for (const auto &[dishId, data] : salesStatistics)
        {
            cout << "Dish " << dishId << ":\n";
            for (const auto &[hour, count] : data.timeCount)
            {
                cout << "  Hour " << hour << ": " << count << " times\n";
            }
        }
        break;
    }
    case 0:
        cout << "Exiting Statistics Menu.\n";
        break;
    default:
        cout << "Invalid choice. Returning to Admin Menu.\n";
    }
}

void User::viewMenu(const Menu &menu)
{
    int choice;
    cout << "1. View Appetizers\n2. View Main Course\n3. View Desserts\n4. View Beverages\n5. Nourishing Meals \n6. View Full Menu\n0. Exit\nEnter choice: ";
    cin >> choice;

    switch (choice)
    {
    case 1:
        menu.showMenu("Appetizers");
        break;
    case 2:
        menu.showMenu("Main Course");
        break;
    case 3:
        menu.showMenu("Desserts");
        break;
    case 4:
        menu.showMenu("Beverages");
        break;
    case 5:
        menu.showMenu("Nourishing Meals");
        break;
    case 6:
        menu.showMenu(); // Show full menu without filtering by category.
        break;
    case 0:
        cout << "Exiting...\n";
        break;
    default:
        cout << "Invalid choice!\n";
    }
}

Order User::placeOrder(Menu &menu, Inventory &inventory, int &chefCounter)
{
    Order order;
    int serialNumber;
    cout << "Enter serial number of items to order (0 to finish):\n";

    while (true)
    {
        cin >> serialNumber;
        if (serialNumber == 0)
            break;

        MenuItem *item = menu.getItem(serialNumber);
        if (item)
        {
            order.items.push_back(*item);
            order.totalCost += item->price;
            item->demandCount++;
        }
        else
        {
            cout << "Item not found!\n";
        }
    }

    order.chefId = chefCounter % 10; // Allocate chef in round-robin
    chefCounter++;

    cout << "Order placed with Chef ID: " << order.chefId << "\n";
    cout << "\n----- Billing Details -----\n";

    if (order.totalCost > 1500 && order.totalCost < 5000)
    {
        double discount = order.totalCost * 0.09;
        double discountedTotal = order.totalCost - discount;
        cout << "Congratulations! You are eligible for a 2% discount.\n";
        cout << "Discount: Rs " << discount << "\n";
        cout << "Total: Rs " << discountedTotal << "\n";
        order.totalCost = discountedTotal;
        cout << "Your total would be: " << order.totalCost - discount << endl;
    }
    else if (order.totalCost > 1500)
    {
        double discount = order.totalCost * 0.09;
        double discountedTotal = order.totalCost - discount;
        cout << "Congratulations! You are eligible for a 2% discount.\n";
        cout << "Discount: Rs " << discount << "\n";
        cout << " Total: Rs " << discountedTotal << "\n";
        order.totalCost = discountedTotal;
        cout << "Your total would be: " << order.totalCost - discount << endl;
    }
    else
    {
        cout << "Your total would be: " << order.totalCost << endl;
    }

    return order;
}
// Show billing details
    cout << "\n----- Billing Details -----\n";
    for (const auto& item : order.items) {
        cout << item.name << " x 1 : Rs " << item.price << "\n";
    }
    cout << "----------------------------\n";
    cout << "Total Cost: Rs " << order.totalCost << "\n";

    // Check and apply offer
    if (order.totalCost > 1000) {
        double discount = order.totalCost * 0.02;
        double discountedTotal = order.totalCost - discount;
        cout << "Congratulations! You are eligible for a 2% discount.\n";
        cout << "Discount: Rs " << discount << "\n";
        cout << "Discounted Total: Rs " << discountedTotal << "\n";
        order.totalCost = discountedTotal;
    }

    cout << "Order placed with Chef ID: " << order.chefId << "\n";
    return order;
}

void User::makeReservation(vector<Reservation> &reservations, unordered_set<int> &tableAvailability)
{
    Reservation res;
    int table;

    cout << "\nAvailable tables: ";

    for (int i = 1; i <= 20; i++)
    {
        if (tableAvailability.find(i) == tableAvailability.end())
        {
            cout << i << " ";
        }
    }

    cout << "\nEnter name for reservation: ";
    cin >> res.name;

    cout << "Enter table numbers (separate by space, end with 0): ";
    while (cin >> table && table != 0)
    {
        if (tableAvailability.find(table) == tableAvailability.end())
        {
            res.tableNumbers.push_back(table);
            tableAvailability.insert(table);
        }
        else
        {
            cout << "Table " << table << " is already reserved!\n";
        }
    }

    if (!res.tableNumbers.empty())
    {
        reservations.push_back(res);
        cout << "Reservation successful for " << res.name << "!\n";
    }
    else
    {
        cout << "No tables reserved. Reservation cancelled.\n";
    }
}

void Restaurant::loadMenu(const string &filename)
{
    menu.loadMenu(filename);
}

void Restaurant::loadInventory(const string &filename)
{
    admin.getInventory().loadInventory(filename); // Load inventory from a JSON file using accessor method.
}

void Restaurant::loadReservationsFromFile()
{
    ifstream file("reservations.txt");
    if (!file)
    {
        cout << "No previous reservations found.\n";
        return;
    }

    string name;
    while (file >> name)
    {
        Reservation res;
        res.name = name;
        int table;
        while (file >> table && table != -1)
        {
            res.tableNumbers.push_back(table);
            tableAvailability.insert(table);
        }
        reservations.push_back(res);
    }
}

void Restaurant::saveReservationsToFile()
{
    ofstream file("reservations.txt");
    for (auto &res : reservations)
    {
        file << res.name << " ";
        for (int table : res.tableNumbers)
        {
            file << table << " ";
        }
        file << "-1\n";
    }
}

void Restaurant::userInterface()
{
    User user;
    int choice;

    do
    {
        cout << "\nUser Interface:\n";
        cout << "1. Menu\n2. Place Order\n3. Make Reservation\n4. Cancel Reservation\n0. Exit\nEnter choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            user.viewMenu(menu);
            break; // Show menu.
        case 2:
        {

            Order order = user.placeOrder(menu, admin.getInventory(), chefCounter);
            orders.push_back(order);

            // Update sales statistics
            time_t now = time(0);
            tm *localTime = localtime(&now);
            int hour = localTime->tm_hour;
            string weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
            string weekday = weekdays[localTime->tm_wday];

            for (const auto &item : order.items)
            {
                salesStatistics[item.serialNumber].timeCount[hour]++;
                salesStatistics[item.serialNumber].weekdayCount[weekday]++;
            }


            break;
        } // Place order.
        case 3:
            user.makeReservation(reservations, tableAvailability);
            break; // Make a reservation.
        case 4:
            cancelReservation(reservations, tableAvailability);
            break; // Cancel a reservation.
        case 0:
            saveReservationsToFile();
            break; // Exit.
        default:
            cout << "Invalid choice!\n";
        }
    } while (choice != 0);
}

void Restaurant::adminInterface()
{
    if (!admin.login())
    {
        cout << "Invalid password!\n";
        return;
    }

    int choice;
    do
    {
        cout << "\nAdmin Interface:\n";
        cout << "1. Manage Inventory\n";
        cout << "2. Manage Menu\n";
        cout << "3. View Statistics\n";
        cout << "4. View Reservations\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            admin.manageInventory();
            break;
        case 2:
            admin.manageMenu(menu);
            break;
        case 3:
            admin.viewStatistics(menu, salesStatistics); // Pass statistics
            break;
        case 4:
            admin.viewReservations(reservations);
            break;
        case 0:
            saveReservationsToFile();
            break;
        default:
            cout << "Invalid choice!\n";
        }
    } while (choice != 0);
}

int main()
{
    Restaurant restaurant("The Gourmet Spot");

    restaurant.loadMenu("menu.json");           // Load the menu from a JSON file.
    restaurant.loadInventory("inventory.json"); // Load the inventory from a JSON file.
    restaurant.loadReservationsFromFile();

    restaurant.loadStatisticsFromFile("statistics.txt");
    int choice;

    do
    {
        cout << "Main Interface:\n";
        cout << "1. User Interface\n2. Admin Interface\n0. Exit\nEnter choice:";
        cin >> choice;

        switch (choice)
        {
        case 1:
            restaurant.userInterface();
            break;
        case 2:
            restaurant.adminInterface();
            break;
        case 0:
            cout << "Goodbye!\n";
            break;
        default:
            cout << "Invalid choice!\n";
        }
    } while (choice != 0);

    // Before exiting
    restaurant.saveStatisticsToFile("statistics.txt");

    return 0;
}

// how to run the program
//  open the terminal and write the following commands

// g++ -I./rapidjson/include main.cpp -o WorldOnAPlate
// ./WorldOnAPlate
