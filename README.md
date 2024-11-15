# World On A Plate - Restaurant Management System

This is a simple restaurant management system that allows both users and admins to interact with the restaurant's operations. Users can place orders, make reservations, and view the menu, while admins can manage the inventory, menu, and reservations.

## Features

### User Features:
- **View Menu**: Users can view the restaurant's menu with item descriptions and prices.
- **Place Orders**: Users can place an order by selecting items from the menu. Each order is assigned to a chef using a round-robin system.
- **Make Reservations**: Users can reserve one or more tables. The available tables are displayed, and the reservations are saved to a text file for persistence.

### Admin Features:
- **Login**: Admins can log in with a password.
- **Manage Menu**: Admins can add, remove, or modify menu items.
- **Manage Inventory**: Admins can view and update the restaurant's inventory.
- **View Reservations**: Admins can view all current reservations.
- **View Statistics**: Admins can view the number of times each menu item has been ordered.

### Additional Features:
- **Chef Allocation**: Orders are assigned to chefs using a round-robin system.
- **Reservation Persistence**: Reservations are stored in a text file, so the data persists between sessions.

## Requirements

- C++ compiler (e.g., GCC, MSVC)
- `rapidjson` library for handling JSON files (for menu loading)
- A text editor or IDE to modify and compile the code

## Installation

1. Clone the repository:

    ```bash
    git clone <repository-url>
    cd World-On-A-Plate
    ```

2. Install `rapidjson` if it’s not already included in your project. You can download it from [rapidjson GitHub](https://github.com/Tencent/rapidjson).

3. Compile the program using your preferred C++ compiler. Example with `g++`:

    ```bash
    g++ main.cpp -o restaurant
    ```
    ```g++ -I./rapidjson/include main.cpp -o WorldOnAPlate```

4. Run the program:

    ```bash
    ./restaurant
    ```
    
    ```./WorldOnAPlate```

## File Structure

- `main.cpp`: Contains the main logic for managing users, orders, reservations, and the admin interface.
- `menu.json`: A sample JSON file that contains the restaurant's menu data.
- `reservations.txt`: A text file used to store reservations.

## Usage

### User Interface

- When prompted, select **1** to enter the user interface.
- You can then:
  - View the menu.
  - Place an order.
  - Make a reservation by selecting available tables.

### Admin Interface

- When prompted, select **2** to enter the admin interface.
- You can then:
  - Manage the menu (add/remove/modify items).
  - View and manage inventory.
  - View statistics on menu item orders.
  - View all reservations.

### Example Flow

1. **User**:
   - View the menu.
   - Place an order by selecting items.
   - Reserve tables (up to 20) under your name.
   
2. **Admin**:
   - Log in with the password.
   - Manage the menu and inventory.
   - View reservations and statistics on menu item orders.

## Notes

- Reservations are limited to 20 tables.
- Admin's password is hardcoded as `"admin123"`.
- The reservation data is saved in `reservations.txt`.
