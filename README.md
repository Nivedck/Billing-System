# Billing System

A simple and efficient billing system designed for small to medium-sized businesses. Built with C++ and the Qt framework, it provides a user-friendly interface for managing products and processing sales.

![Screenshot](resources/screenshot.png)

## Features

*   **Product Management:** Easily add, update, and delete products, including managing their stock levels, through a secure admin panel.
*   **Secure Admin Login:** Access to the admin panel is now protected by a robust login system with user management capabilities.
*   **Admin User Management:** Create and delete administrator accounts directly from within the admin panel.
*   **Stock Management:** Products now have a stock quantity, which is automatically decremented upon purchase. Stock levels are visible and manageable in the admin panel.
*   **Clear Database Option:** A powerful option in the admin panel to clear all transactional and product data, allowing for a fresh start.
*   **Billing:** A streamlined process for adding products to a cart, calculating the total cost (including tax), and generating a bill.
*   **Search with Autocomplete:** Quickly find products by name with the help of an autocomplete feature.
*   **User-Friendly Interface:** A clean and intuitive graphical user interface built with Qt.

## Getting Started

### Prerequisites

*   C++ compiler (g++, clang, etc.)
*   Qt 6 or Qt 5 (QtWidgets)
*   SQLite

### Building and Running

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/Nivedck/Billing-System.git
    cd Billing-System
    ```

2.  **Open in Qt Creator:**
    *   Open the `Billing-System.pro` file in Qt Creator.
    *   Configure the project with your Qt kit.
    *   Build and run the project.

3.  **Command line (with qmake):**
    ```bash
    qmake
    make
    ./Billing-System
    ```

## How to Use

### Admin Panel

1.  Click the "Admin Panel" button on the main window.
2.  **Login:** Enter your admin username and password in the login dialog. The default credentials for the first run are `username: admin`, `password: admin`.
3.  Once logged in, you can:
    *   **Product Management:** Add new products, update existing product details (including name, price, and stock quantity), or delete products.
    *   **Admin Management:** Switch to the "Admin Management" tab to add new admin users or delete existing ones.
    *   **Clear Database:** Use the "Clear All Database Data" button with caution to reset all product, sales, and admin data (a default admin will be re-created).

### Billing

1.  **Add products to the cart:**
    *   Enter the product name in the "Product Name" field. The autocomplete feature will suggest matching products.
    *   Enter the quantity and press "Add to Cart". The system will check for available stock.
2.  **Manage the cart:**
    *   The cart contents are displayed in a table.
    *   To remove an item, select it in the table and click "Remove Item".
    *   To clear the entire cart, click "Clear Cart".
3.  **Checkout:**
    *   Click "Checkout" to finalize the sale. The stock quantity for purchased items will be automatically updated.
    *   A bill will be displayed with the total amount.

## Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, please open an issue or create a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.