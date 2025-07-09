# Billing System

A simple and efficient billing system designed for small to medium-sized businesses. Built with C++ and the Qt framework, it provides a user-friendly interface for managing products and processing sales.

![Screenshot](resources/screenshot.png)

## Features

*   **Product Management:** Easily add, update, and delete products from the database through a secure admin panel.
*   **Billing:** A streamlined process for adding products to a cart, calculating the total cost (including tax), and generating a bill.
*   **Search with Autocomplete:** Quickly find products by name with the help of an autocomplete feature.
*   **Database Integration:** Uses SQLite to store and manage product information.
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

1.  Click the "Admin" button on the main window.
2.  Enter the admin password (the default is `nivedck`) to access the admin panel.
3.  In the admin panel, you can:
    *   **Add a product:** Fill in the product code, name, and price, then click "Add".
    *   **Update a product:** Enter the product code of the item you want to update, fill in the new name and price, then click "Update".
    *   **Delete a product:** Enter the product code of the item you want to delete, then click "Delete".

### Billing

1.  **Add products to the cart:**
    *   Enter the product name in the "Product Name" field. The autocomplete feature will suggest matching products.
    *   Enter the quantity and press "Add to Cart".
2.  **Manage the cart:**
    *   The cart contents are displayed in a table.
    *   To remove an item, select it in the table and click "Remove".
    *   To clear the entire cart, click "Clear Cart".
3.  **Checkout:**
    *   Click "Checkout" to finalize the sale.
    *   A bill will be displayed with the total amount.

## Contributing

Contributions are welcome! If you have any ideas, suggestions, or bug reports, please open an issue or create a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.