#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include "rapidjson/writer.h"
#include <algorithm>

using namespace std;
using namespace rapidjson;

// Struct for MenuItem
struct MenuItem {
    int serialNumber;
    string category;
    string description;
    double price;
    int demandCount = 0; // To track popularity
};

// Struct for Order
struct Order {
    int chefId;
    vector<MenuItem> items;
    double totalCost = 0;
};

// Struct for Reservation
struct Reservation {
    string name;
    vector<int> tableNumbers; // Allows multiple tables per reservation
};

class Menu {
    vector<MenuItem> items;
public:
    void loadMenu(const string& filename);
    void showMenu(const string& category = "") const; // Updated to filter by category
    void addItem(const MenuItem& item);
    void removeItem(int serialNumber);
    void modifyItem(int serialNumber, const MenuItem& item);
    MenuItem* getItem(int serialNumber);
    const vector<MenuItem>& getItems() const {
        return items;
    }
};

class Inventory {
    unordered_map<string, int> stock; // Material name and quantity
public:
    void showInventory();
    void updateInventory(const string& material, int quantity);
};

// Admin class
class Admin {
    string password = "admin123";
    Inventory inventory;
public:
    bool login();
    void manageInventory();
    void manageMenu(Menu& menu);
    void viewStatistics(const Menu& menu);
    void viewReservations(const vector<Reservation>& reservations);
};

// User class
class User {
    string name;
public:
    void viewMenu(const Menu& menu);
    Order placeOrder(Menu& menu, int& chefCounter);
    void makeReservation(vector<Reservation>& reservations, unordered_set<int>& tableAvailability);
};

class Restaurant {
    string name;
    Menu menu;
    vector<Reservation> reservations;
    Admin admin;
    vector<Order> orders;
    int chefCounter = 0; // Round-robin chef allocation
public:
    Restaurant(const string& restaurantName) : name(restaurantName) {}
    void loadMenu(const string& filename);
    void loadReservationsFromFile();
    void saveReservationsToFile();
    void userInterface();
    void adminInterface();
};

// Method Implementations

void Inventory::showInventory() {
    cout << "Inventory Levels:\n";
    for (const auto& item : stock) {
        cout << item.first << ": " << item.second << "\n";
    }
}

void Inventory::updateInventory(const string& material, int quantity) {
    stock[material] = quantity;
}

void Menu::loadMenu(const string& filename) {
    FILE* fp = fopen(filename.c_str(), "r");
    if (!fp) {
        cerr << "Failed to open menu file.\n";
        return;
    }
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    Document doc;
    doc.ParseStream(is);
    fclose(fp);

    for (auto& item : doc.GetArray()) {
        MenuItem menuItem;
        menuItem.serialNumber = item["serialNumber"].GetInt();
        menuItem.category = item["category"].GetString();
        menuItem.description = item["description"].GetString();
        menuItem.price = item["price"].GetDouble();
        items.push_back(menuItem);
    }
}

void Menu::showMenu(const string& category) const {
    cout << "Menu:\n";
    bool categoryFound = false;

    for (const auto& item : items) {
        if (category.empty() || item.category == category) {
            cout << item.serialNumber << " - " << item.category << " - " 
                 << item.description << " - $" << item.price << "\n";
            categoryFound = true;
        }
    }

    if (!category.empty() && !categoryFound) {
        cout << "No items found for the category: " << category << "\n";
    }
}

void Menu::addItem(const MenuItem& item) {
    items.push_back(item);
}

void Menu::removeItem(int serialNumber) {
    items.erase(remove_if(items.begin(), items.end(),
                          [serialNumber](const MenuItem& item) { return item.serialNumber == serialNumber; }),
                items.end());
}

void Menu::modifyItem(int serialNumber, const MenuItem& updatedItem) {
    for (auto& item : items) {
        if (item.serialNumber == serialNumber) {
            item = updatedItem;
            return;
        }
    }
}

MenuItem* Menu::getItem(int serialNumber) {
    for (auto& item : items) {
        if (item.serialNumber == serialNumber) return &item;
    }
    return nullptr;
}

bool Admin::login() {
    string inputPassword;
    cout << "Enter admin password: ";
    cin >> inputPassword;
    return inputPassword == password;
}

void Admin::manageInventory() {
    inventory.showInventory();
    string material;
    int quantity;
    cout << "Enter material and quantity to update (e.g., 'Flour 100'): ";
    cin >> material >> quantity;
    inventory.updateInventory(material, quantity);
}

void Admin::manageMenu(Menu& menu) {
    int choice;
    cout << "1. Add Item\n2. Remove Item\n3. Modify Item\nEnter choice: ";
    cin >> choice;

    if (choice == 1) {
        MenuItem item;
        cout << "Enter serial number, category, description, and price: ";
        cin >> item.serialNumber >> item.category >> item.description >> item.price;
        menu.addItem(item);
    } else if (choice == 2) {
        int serialNumber;
        cout << "Enter serial number to remove: ";
        cin >> serialNumber;
        menu.removeItem(serialNumber);
    } else if (choice == 3) {
        int serialNumber;
        cout << "Enter serial number to modify: ";
        cin >> serialNumber;
        MenuItem item;
        cout << "Enter updated category, description, and price: ";
        cin >> item.category >> item.description >> item.price;
        menu.modifyItem(serialNumber, item);
    }
}

void Admin::viewStatistics(const Menu& menu) {
    cout << "Statistics:\n";
    for (const auto& item : menu.getItems()) {
        cout << item.description << " sold " << item.demandCount << " times.\n";
    }
}

void Admin::viewReservations(const vector<Reservation>& reservations) {
    cout << "Reservations:\n";
    for (const auto& res : reservations) {
        cout << res.name << " reserved tables: ";
        for (int table : res.tableNumbers) {
            cout << table << " ";
        }
        cout << "\n";
    }
}

void User::viewMenu(const Menu& menu) {
    menu.showMenu();
}

Order User::placeOrder(Menu& menu, int& chefCounter) {
    Order order;
    int serialNumber;
    cout << "Enter serial number of items to order (0 to finish):\n";
    while (true) {
        cin >> serialNumber;
        if (serialNumber == 0) break;
        MenuItem* item = menu.getItem(serialNumber);
        if (item) {
            order.items.push_back(*item);
            order.totalCost += item->price;
            item->demandCount++;
        } else {
            cout << "Item not found!\n";
        }
    }
    order.chefId = chefCounter % 10; // Allocate chef in round-robin
    chefCounter++;
    cout << "Order placed with Chef ID: " << order.chefId << "\n";
    return order;
}

void User::makeReservation(vector<Reservation>& reservations, unordered_set<int>& tableAvailability) {
    Reservation res;
    int table;
    cout << "\nAvailable tables: ";
    for (int i = 1; i <= 20; i++) {
        if (tableAvailability.find(i) == tableAvailability.end()) {
            cout << i << " ";
        }
    }
    cout << "\nEnter name for reservation: ";
    cin >> res.name;
    cout << "Enter table numbers (separate by space, end with 0): ";
    while (cin >> table && table != 0) {
        if (tableAvailability.find(table) == tableAvailability.end()) {
            res.tableNumbers.push_back(table);
            tableAvailability.insert(table);
        } else {
            cout << "Table " << table << " is already reserved!\n";
        }
    }
    reservations.push_back(res);
}

void Restaurant::loadMenu(const string& filename) {
    menu.loadMenu(filename);
}

void Restaurant::loadReservationsFromFile() {
    ifstream file("reservations.txt");
    if (!file) {
        cout << "No previous reservations found.\n";
        return;
    }
    string name;
    while (file >> name) {
        Reservation res;
        res.name = name;
        int table;
        while (file >> table && table != -1) {
            res.tableNumbers.push_back(table);
        }
        reservations.push_back(res);
    }
}

void Restaurant::saveReservationsToFile() {
    ofstream file("reservations.txt");
    for (const auto& res : reservations) {
        file << res.name << " ";
        for (int table : res.tableNumbers) {
            file << table << " ";
        }
        file << "-1\n";
    }
}

void Restaurant::userInterface() {
    User user;
    int choice;
    while (true) {
        cout << "\nUser Options:\n";
        cout << "1. View Full Menu\n";
        cout << "2. View Appetizers\n";
        cout << "3. View Main Course\n";
        cout << "4. View Desserts\n";
        cout << "5. View Beverages\n";
        cout << "6. Make Reservation\n";
        cout << "7. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) {
            menu.showMenu();
        } else if (choice == 2) {
            menu.showMenu("Appetizers");
        } else if (choice == 3) {
            menu.showMenu("Main Course");
        } else if (choice == 4) {
            menu.showMenu("Desserts");
        } else if (choice == 5) {
            menu.showMenu("Beverages");
        } else if (choice == 6) {
            unordered_set<int> tableAvailability;
            user.makeReservation(reservations, tableAvailability);
        } else if (choice == 7) {
            break;
        }
    }
}

void Restaurant::adminInterface() {
    if (!admin.login()) {
        cout << "Invalid password!\n";
        return;
    }
    int choice;
    while (true) {
        cout << "1. Manage Inventory\n2. Manage Menu\n3. View Statistics\n4. View Reservations\n5. Exit\nEnter choice: ";
        cin >> choice;
        if (choice == 1) {
            admin.manageInventory();
        } else if (choice == 2) {
            admin.manageMenu(menu);
        } else if (choice == 3) {
            admin.viewStatistics(menu);
        } else if (choice == 4) {
            admin.viewReservations(reservations);
        } else if (choice == 5) {
            break;
        }
    }
}

int main() {
    Restaurant restaurant("World On A Plate");

    // Load menu and previous reservations
    restaurant.loadMenu("menu.json");
    restaurant.loadReservationsFromFile();

    // User/Admin interface
    int choice;
    while (true) {
        cout << "1. User Interface\n2. Admin Interface\n3. Exit\nEnter choice: ";
        cin >> choice;
        if (choice == 1) {
            restaurant.userInterface();
        } else if (choice == 2) {
            restaurant.adminInterface();
        } else {
            break;
        }
    }

    // Save reservations
    restaurant.saveReservationsToFile();

    return 0;
}


//how to run the program 
// open the terminal and write the following commands 

// g++ -I./rapidjson/include main.cpp -o WorldOnAPlate
// ./WorldOnAPlate