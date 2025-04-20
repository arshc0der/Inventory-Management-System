
---

# Initial Testing Guide for IMS Software

## Overview
This guide provides steps to set up and test the IMS (Inventory Management System) software. The process involves loading a test database file, logging in as both an Admin and an Employee, and testing the functionalities of the software.

## Prerequisites
- Ensure you have the software installed on your system.
- Ensure you have access to the necessary GitHub repository to download the database file.

## Steps to Perform Initial Testing

### 1. Setup the Test Database
1. Open the software once to initialize it.
2. Navigate to the `%APPDATA%` folder on your system. This can be done by typing `%APPDATA%` in the File Explorer address bar.
3. Find the folder named `IMS` within the `%APPDATA%` directory.
4. Inside the `IMS` folder, locate and delete the file `i_m_s.db`.

### 2. Download the Test Database
1. Download the latest version of the `i_m_s.db` file from the repository.
 - [Download the i_m_s.db file from GitHub](https://github.com/arshc0der/Inventory-Management-System/blob/main/docs/Tester/Testing_Database_File/i_m_s.db)
2. Copy the downloaded file into the `IMS` folder in `%APPDATA%`.

### 3. Verify the Database is Loaded
1. Open the IMS software again.
2. The software should load the newly copied database (`i_m_s.db`).

### 4. Test Login Functionality

#### Admin Login
1. From the top navigation, switch to the Admin login page.
2. Use the following credentials to log in:
   - **Email:** `test@test.com`
   - **Username:** `admin`
   - **Password:** `adminUSA`
   - **Forget Pin:** `1234`
3. Verify that you are logged in as the Admin user.

#### Employee Login
1. From the top navigation, switch to the Employee login page.
2. Use the following temporary credentials to log in:
   - **Email:** `roger@test.com`
   - **Temporary Password:** `1450dee3`
3. You may be prompted to set a new password. Set the password according to the instructions.

### 5. Switch Between Admin and Employee
1. Once logged in as Admin, verify that you can switch between the Admin and Employee roles using the navigation options.
2. Test the Admin and Employee functionalities to ensure proper access levels and features are working.

### 6. Final Steps
- Perform any further testing based on the requirements and functionality of the software.
- Report any issues or bugs to the development team for review.

## Conclusion
This guide outlines the steps for initial testing of the IMS software. Ensure that the login functionality works correctly for both Admin and Employee users and that the software properly loads the test database.

---
