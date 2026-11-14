#!/usr/bin/env python3
"""
Generate a realistic demo / test database for IMS.

    python3 tools/generate_test_db.py [output.db]

It builds the schema straight from DatabaseSchema.h (so it can never drift from the
app), then seeds suppliers, customers, ~60 products, sales, orders, adjustments,
promotions, employees, notifications and an audit trail spread over the last 60 days.
Stock levels are computed so the final quantities include a healthy mix of
in-stock / low-stock / out-of-stock products.

Logins (SHA-256(password + salt), same scheme as the app):
    Admin     username: admin             password: Admin@123    (admin page, PIN 1234)
    Employee  email: priya@northwind.test password: Emp@12345    (employee page)
"""
import hashlib
import os
import random
import re
import sqlite3
import sys
from datetime import date, datetime, timedelta

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    ROOT, "docs", "Tester", "Testing_Database_File", "i_m_s_demo.db")

random.seed(2026)
TODAY = date.today()


def sha(password, salt):
    return hashlib.sha256((password + salt).encode("utf-8")).hexdigest()


def d(days_ago):
    return (TODAY - timedelta(days=days_ago)).isoformat()


def ts(days_ago, hour=None):
    h = hour if hour is not None else random.randint(8, 18)
    return (datetime.combine(TODAY, datetime.min.time()) - timedelta(days=days_ago)
            + timedelta(hours=h, minutes=random.randint(0, 59))).strftime("%Y-%m-%d %H:%M:%S")


if os.path.exists(OUT):
    os.remove(OUT)
os.makedirs(os.path.dirname(OUT), exist_ok=True)
db = sqlite3.connect(OUT)
db.execute("PRAGMA foreign_keys = OFF")

# ---------------------------------------------------------------- schema
schema = open(os.path.join(ROOT, "DatabaseSchema.h"), encoding="utf8").read()
for block in re.findall(r'R"\((.*?)\)"', schema, re.S):
    db.executescript(block)

# ---------------------------------------------------------------- users
admin_salt = "a1b2c3d4e5f6"
db.execute(
    "INSERT INTO users (business_name,address,email,pin,name,username,password_hash,password_salt,role,"
    "is_active,theme_preference) VALUES (?,?,?,?,?,?,?,?,?,?,?)",
    ("Northwind Traders", "221 Market Street, Noida, UP 201301", "admin@northwind.test", 1234,
     "Arsh Chaudhary", "admin", sha("Admin@123", admin_salt), admin_salt, "Admin", 1, "light"))
admin_id = db.execute("SELECT id FROM users WHERE username='admin'").fetchone()[0]

perm_cols = [r[1] for r in db.execute("PRAGMA table_info(User_permissions)") if r[1] != "user_id"]
db.execute("INSERT INTO User_permissions (user_id,%s) VALUES (?,%s)" %
           (",".join(perm_cols), ",".join("1" * len(perm_cols))), (admin_id,))

# ---------------------------------------------------------------- suppliers / customers
suppliers = [
    ("Apex Electronics Ltd", "sales@apex-electronics.test | +91 98100 11001", "Sector 62, Noida"),
    ("BrightHome Supplies", "orders@brighthome.test | +91 98100 11002", "Okhla Phase 2, New Delhi"),
    ("Cascade Office Co", "hello@cascadeoffice.test | +91 98100 11003", "MG Road, Gurugram"),
    ("Delta Fresh Foods", "b2b@deltafresh.test | +91 98100 11004", "Azadpur Mandi, Delhi"),
    ("Evergreen Stationers", "team@evergreen.test | +91 98100 11005", "Connaught Place, New Delhi"),
    ("Futura Tools & Hardware", "sales@futuratools.test | +91 98100 11006", "Ballabgarh, Faridabad"),
    ("GreenLeaf Wellness", "care@greenleaf.test | +91 98100 11007", "Sector 18, Noida"),
    ("Helix Mobility", "supply@helixmobility.test | +91 98100 11008", "Bhiwadi, Rajasthan"),
]
for s in suppliers:
    db.execute("INSERT INTO suppliers (name,contact_info,address) VALUES (?,?,?)", s)
sup_ids = [r[0] for r in db.execute("SELECT supplier_id FROM suppliers ORDER BY supplier_id")]

first = ["Aarav", "Diya", "Kabir", "Meera", "Rohan", "Ananya", "Vikram", "Ishita", "Arjun", "Neha",
         "Siddharth", "Kavya", "Raj", "Tanvi", "Yash"]
last = ["Sharma", "Verma", "Singh", "Gupta", "Mehta", "Iyer", "Kapoor", "Nair", "Reddy", "Bose"]
for i, f in enumerate(first):
    n = "%s %s" % (f, random.choice(last))
    db.execute("INSERT INTO customers (name,contact_info,address,loyalty_points) VALUES (?,?,?,?)",
               (n, "%s@mail.test | +91 9%09d" % (f.lower(), random.randint(10**8, 10**9 - 1)),
                "%d, %s Nagar" % (random.randint(1, 120), random.choice(["Rajouri", "Vasant", "Saket", "Indira"])),
                random.randint(0, 900)))
cust_ids = [r[0] for r in db.execute("SELECT customer_id FROM customers")]

# ---------------------------------------------------------------- products
catalog = {
    "Electronics": [("Wireless Mouse", 799), ("Mechanical Keyboard", 3499), ("USB-C Hub 7-in-1", 2199),
                    ("27\" Monitor", 15999), ("Webcam 1080p", 2799), ("Bluetooth Speaker", 1899),
                    ("Power Bank 20000mAh", 1799), ("Noise-Cancelling Headset", 6999)],
    "Home & Kitchen": [("Air Fryer 4L", 5499), ("Electric Kettle", 1299), ("Non-stick Pan Set", 2499),
                       ("Water Bottle 1L", 399), ("LED Desk Lamp", 999), ("Storage Box Set", 899),
                       ("Cotton Towel Pack", 749), ("Ceramic Mug Set", 599)],
    "Office": [("A4 Paper Ream", 349), ("Gel Pen Box (50)", 450), ("Stapler Heavy Duty", 299),
               ("Desk Organizer", 649), ("Whiteboard 3x2", 1799), ("Sticky Notes Pack", 199),
               ("Ergonomic Chair", 8999), ("Laser Printer", 11999)],
    "Grocery": [("Basmati Rice 5kg", 749), ("Cold-press Olive Oil 1L", 899), ("Green Tea 100 bags", 349),
                ("Almonds 500g", 549), ("Organic Honey 500g", 499), ("Instant Coffee 200g", 449),
                ("Whole Wheat Flour 10kg", 599), ("Dark Chocolate Box", 399)],
    "Tools": [("Cordless Drill 18V", 4999), ("Screwdriver Set 32pc", 899), ("Tape Measure 5m", 249),
              ("Safety Gloves Pair", 199), ("Toolbox 19in", 1299), ("Spirit Level 24in", 549),
              ("Adjustable Wrench", 449), ("Hammer 500g", 399)],
    "Wellness": [("Yoga Mat 6mm", 999), ("Resistance Bands Set", 699), ("Vitamin C 60 tabs", 399),
                 ("Digital Thermometer", 549), ("Massage Gun", 3999), ("Protein Powder 1kg", 2499),
                 ("Fitness Tracker", 2999), ("Foam Roller", 899)],
    "Mobility": [("Helmet ISI Full Face", 2299), ("Bike Phone Mount", 499), ("Tyre Inflator", 1599),
                 ("Car Vacuum Cleaner", 1999), ("Dash Camera", 4499), ("Seat Cover Set", 2799),
                 ("Jump Starter", 5999), ("LED Headlight Pair", 1799)],
}
products = []  # dict(name,cat,price,min,target_final,supplier)
cat_supplier = dict(zip(catalog.keys(), [0, 1, 2, 3, 5, 6, 7]))
for cat, items in catalog.items():
    for name, price in items:
        minimum = random.choice([5, 8, 10, 12, 15, 20])
        roll = random.random()
        if roll < 0.10:
            final = 0                                   # out of stock
        elif roll < 0.28:
            final = random.randint(1, minimum)          # low stock
        else:
            final = random.randint(minimum + 5, minimum + 90)
        products.append(dict(name=name, cat=cat, price=float(price), min=minimum, final=final,
                             supplier=sup_ids[cat_supplier[cat]], consumed=0, adds=0, id=None,
                             bought=random.randint(20, 90)))

# plan consumption first, so initial stock = final + everything that leaves
# ---- sales plan
sales_plan = []
for _ in range(150):
    p = random.choice(products)
    qty = random.randint(1, 6)
    ago = random.randint(0, 59)
    disc = random.choice([0, 0, 0, 5, 10, 15])
    sales_plan.append((p, qty, ago, disc))
    p["consumed"] += qty

# ---- orders plan
statuses = ["Completed"] * 6 + ["Pending"] * 3 + ["Cancelled"] * 2
orders_plan = []
for _ in range(60):
    status = random.choice(statuses)
    lines = random.sample(products, random.randint(1, 4))
    detail = [(p, random.randint(1, 5)) for p in lines]
    if status == "Completed":
        for p, q in detail:
            p["consumed"] += q
    orders_plan.append((status, random.randint(0, 59), random.choice(cust_ids), detail))

# ---- adjustments plan
adj_plan = []
for _ in range(25):
    p = random.choice(products)
    typ = random.choice(["Add", "Add", "Remove"])
    qty = random.randint(2, 12)
    reason = random.choice(["Supplier restock", "Cycle count correction", "Damaged in storage",
                            "Customer return", "Expired stock write-off", "Transfer from warehouse B"])
    adj_plan.append((p, typ, qty, random.randint(0, 59), reason))
    if typ == "Remove":
        p["consumed"] += qty
    else:
        p["adds"] += qty

for p in products:
    # adjustments "Add" run first, so initial stock only has to cover the outgoing quantity
    p["start"] = max(0, p["final"] + p["consumed"] - p["adds"])

for p in products:
    cur = db.execute(
        "INSERT INTO products (name,category,quantity,price,supplier_id,purchase_date,minimum_stock_level,description)"
        " VALUES (?,?,?,?,?,?,?,?)",
        (p["name"], p["cat"], p["start"], p["price"], p["supplier"], d(p["bought"]), p["min"],
         "%s - %s" % (p["cat"], p["name"])))
    p["id"] = cur.lastrowid

# ---------------------------------------------------------------- adjustments
def run_adjustments(kind):
    for p, typ, qty, ago, reason in adj_plan:
        if typ == kind:
            db.execute("INSERT INTO inventory_adjustments (product_id,adjustment_type,quantity,adjustment_date,reason)"
                       " VALUES (?,?,?,?,?)", (p["id"], typ, qty, ts(ago), reason))


run_adjustments("Add")          # stock-ins first
# ---------------------------------------------------------------- sales (trigger reduces stock)
for p, qty, ago, disc in sales_plan:
    total = round(p["price"] * qty * (1 - disc / 100.0), 2)
    db.execute("INSERT INTO sales (product_id,quantity_sold,sale_date,total_price,discount,customer_id)"
               " VALUES (?,?,?,?,?,?)",
               (p["id"], qty, ts(ago), total, disc, random.choice(cust_ids)))

# ---------------------------------------------------------------- orders
for status, ago, cust, detail in orders_plan:
    total = round(sum(p["price"] * q for p, q in detail), 2)
    cur = db.execute("INSERT INTO orders (customer_id,order_date,status,total_price) VALUES (?,?,?,?)",
                     (cust, ts(ago), status, total))
    oid = cur.lastrowid
    for p, q in detail:
        db.execute("INSERT INTO order_details (order_id,product_id,quantity,price) VALUES (?,?,?,?)",
                   (oid, p["id"], q, p["price"]))

run_adjustments("Remove")       # write-offs last

# make sure the app-level invariants hold (some triggers clamp, others do not)
db.execute("UPDATE products SET quantity = 0 WHERE quantity < 0")

# ---------------------------------------------------------------- promotions
promos = [
    ("Diwali Mega Sale", "Festival discount on electronics and home", -10, 20, 25, "Electronics"),
    ("Back to Office", "Stationery and furniture offers", -3, 25, 15, "Office"),
    ("Monsoon Mobility Deals", "Rider safety and car accessories", 5, 35, 12, "Mobility"),
    ("Winter Wellness Week", "Fitness and health essentials", 12, 19, 18, "Wellness"),
    ("Pantry Refill", "Bulk grocery savings", -40, -12, 10, "Grocery"),
    ("Summer Tools Clearance", "Hardware clearance", -75, -45, 30, "Tools"),
]
for name, desc, s, e, pct, cat in promos:
    ids = ",".join(str(p["id"]) for p in products if p["cat"] == cat)
    db.execute("INSERT INTO promotions (name,description,start_date,end_date,discount_percentage,applicable_product_ids)"
               " VALUES (?,?,?,?,?,?)", (name, desc, d(-s), d(-e), pct, ids))

# ---------------------------------------------------------------- employees (+ permissions)
people = [
    ("Priya Sharma", "Store Manager", "Operations", "full-time", 78000, "priya@northwind.test"),
    ("Rahul Verma", "Inventory Analyst", "Inventory", "full-time", 52000, "rahul@northwind.test"),
    ("Sneha Kulkarni", "Sales Executive", "Sales", "full-time", 41000, "sneha@northwind.test"),
    ("Amit Joshi", "Warehouse Lead", "Logistics", "full-time", 46000, "amit@northwind.test"),
    ("Pooja Nair", "Accountant", "Finance", "full-time", 58000, "pooja@northwind.test"),
    ("Karan Malhotra", "Procurement Officer", "Procurement", "full-time", 55000, "karan@northwind.test"),
    ("Divya Menon", "Customer Support", "Support", "part-time", 24000, "divya@northwind.test"),
    ("Imran Qureshi", "Delivery Coordinator", "Logistics", "contract", 36000, "imran@northwind.test"),
    ("Tanya Bhatia", "Marketing Associate", "Marketing", "full-time", 43000, "tanya@northwind.test"),
    ("Suresh Patel", "Security Supervisor", "Facilities", "full-time", 32000, "suresh@northwind.test"),
    ("Lakshmi Rao", "HR Executive", "HR", "full-time", 47000, "lakshmi@northwind.test"),
    ("Naveen Kumar", "Data Entry Operator", "Inventory", "intern", 15000, "naveen@northwind.test"),
]
emp_perm_cols = [r[1] for r in db.execute("PRAGMA table_info(permissions)") if r[1] != "user_id"]
for i, (name, role, dept, typ, sal, email) in enumerate(people):
    salt = "s%02dxyz" % i
    active = 0 if name in ("Naveen Kumar",) else 1
    cur = db.execute(
        "INSERT INTO employees (name,email,phone_number,role,department,hire_date,status,salary,bonus,"
        "employee_type,password_hash,password_salt,is_active,join_date,work_email,shift_timing,employment_status,"
        "force_password_change,address,gender) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
        (name, email, "+91 98%08d" % random.randint(0, 10**8 - 1), role, dept, d(random.randint(90, 1400)),
         "active" if active else "inactive", float(sal), float(random.choice([0, 2000, 5000, 8000])), typ,
         sha("Emp@12345", salt), salt, active, d(random.randint(90, 1400)), email,
         random.choice(["09:00-18:00", "10:00-19:00", "07:00-16:00"]),
         "Employed" if active else "Resigned", 0, "Noida, UP",
         random.choice(["Male", "Female"])))
    eid = cur.lastrowid
    flags = {c: 0 for c in emp_perm_cols}
    if role in ("Store Manager",):
        flags = {c: 1 for c in emp_perm_cols}
    elif role in ("Inventory Analyst", "Warehouse Lead", "Data Entry Operator"):
        for c in ("dashboard_access", "product_dashboard", "add_product", "update_products",
                  "advance_view_products", "adjustment_Stock", "notification_dashboard"):
            flags[c] = 1
    elif role in ("Sales Executive", "Customer Support"):
        for c in ("dashboard_access", "sales_dashboard", "add_sales", "update_sales", "advance_view_sales",
                  "orders_dashboard", "add_orders", "advance_view_orders", "notification_dashboard"):
            flags[c] = 1
    else:
        for c in ("dashboard_access", "product_dashboard", "sales_dashboard", "orders_dashboard",
                  "advance_view_products", "advance_view_sales", "advance_view_orders", "activity_dashboard"):
            flags[c] = 1
    db.execute("INSERT INTO permissions (user_id,%s) VALUES (?,%s)" %
               (",".join(emp_perm_cols), ",".join("?" * len(emp_perm_cols))),
               [eid] + [flags[c] for c in emp_perm_cols])

# ---------------------------------------------------------------- notifications + audit trail
notes = [
    ("Welcome to IMS - demo data loaded.", "info"),
    ("Several products are below their minimum stock level.", "risk"),
    ("Promotion 'Diwali Mega Sale' is now active.", "info"),
    ("Supplier Apex Electronics updated their price list.", "info"),
    ("Out-of-stock items need urgent reordering.", "risk"),
    ("Monthly inventory audit is due this week.", "info"),
    ("Employee Naveen Kumar's account was deactivated.", "info"),
    ("Order backlog: pending orders older than 7 days.", "risk"),
]
for i, (m, t) in enumerate(notes):
    db.execute("INSERT INTO notifications (message,created_at,is_read,type,unique_key,auto_generated)"
               " VALUES (?,?,?,?,?,1)", (m, ts(i * 2), 1 if i > 4 else 0, t, "demo-%d" % i))

actions = [("Login", "User admin logged in."), ("Logout", "User admin logged out."),
           ("Add Product", "Product added by user admin"), ("Add Sale", "Sale added by user admin"),
           ("Update Record", "Record updated by user admin"), ("Export Inventory", "Exported 60 products to CSV"),
           ("Adjust Stock", "Stock adjustment recorded")]
for _ in range(70):
    a, det = random.choice(actions)
    db.execute("INSERT INTO audit_logs (user_id,action,timestamp,details) VALUES (?,?,?,?)",
               (admin_id, a, ts(random.randint(0, 45)), det))

db.commit()

# ---------------------------------------------------------------- report
print("Created", OUT)
for t in ("users", "employees", "suppliers", "customers", "products", "sales", "orders", "order_details",
          "inventory_adjustments", "promotions", "notifications", "audit_logs"):
    print("  %-22s %4d" % (t, db.execute("SELECT COUNT(*) FROM %s" % t).fetchone()[0]))
low = db.execute("SELECT COUNT(*) FROM products WHERE quantity>0 AND quantity<=minimum_stock_level").fetchone()[0]
out = db.execute("SELECT COUNT(*) FROM products WHERE quantity=0").fetchone()[0]
print("  low stock: %d   out of stock: %d" % (low, out))
db.close()
