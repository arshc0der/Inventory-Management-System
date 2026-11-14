# Demo database

`Testing_Database_File/i_m_s_demo.db` is a realistic, fully-populated database for testing IMS.

1. Close IMS.
2. Copy the file to `%APPDATA%\IMS\` and rename it to `i_m_s.db` (replace the existing one).
3. Start IMS and log in:
   * Admin: username `admin`, password `Admin@123`, PIN `1234`
   * Employee: `priya@northwind.test` / `Emp@12345` (manager, all permissions)
   * Employee: `rahul@northwind.test` / `Emp@12345` (inventory only)
   * Employee: `sneha@northwind.test` / `Emp@12345` (sales and orders only)

Regenerate with fresh dates: `python3 tools/generate_test_db.py`.
