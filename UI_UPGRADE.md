# IMS UI Upgrade - Enterprise Design Layer

This release restyles IMS with an IBM Carbon-inspired design system and adds
animation plus several productivity features. All existing screens, database
code and business logic are unchanged; the new layer sits on top.

## What is new

### Design system (`ims_theme.*`)
* Carbon-style **Light** ("White") and **Dark** ("Gray 100") themes, IBM Blue `#0F62FE`.
* One application-wide stylesheet built from design tokens (colours, borders, states).
* Flat cards with hairline borders, bottom-border form fields, square-ish buttons,
  slim scrollbars, styled tables / tabs / menus / tooltips.
* Theme choice is remembered between launches (QSettings, does not touch the DB path).
* The old inline styles in the `.ui` files are removed at runtime the first time each
  window opens, and widgets are tagged with semantic roles (card, nav, primary,
  danger, field, hero ...) that the stylesheet targets. The `.ui` files themselves
  are untouched, so you can still edit them in Qt Designer.
* Charts follow the theme (IBM Blue series, themed axes/grid) and animate on load.

### Animation (`ims_fx.*`)
* Every window fades in; dashboard pages fade + slide up when you switch sections.
* KPI numbers ("Total Products : 1,234") count up from zero.
* KPI cards lift with a soft blue shadow on hover and get a coloured accent stripe.
* Sidebar icons swap to blue on hover / active.
* Failed login shakes the button.
* The splash screen no longer freezes the UI (`QThread::sleep` replaced by an event-loop wait).

### New features
| Feature | How to use |
| --- | --- |
| **Command palette** | `Ctrl+K`, type "prod", Enter. Jump to any page or start Add Product / Sale / Order / Promotion / Stock adjustment. |
| **Inventory Explorer** | New sidebar entry "Inventory". Live search, category filter, low-stock toggle, click-to-sort columns, colour-coded status, suggested reorder quantity, KPI strip. |
| **CSV export** | "Export CSV" in the Explorer exports exactly the rows on screen (UTF-8 with BOM for Excel) and writes an audit-log entry. |
| **Dark mode** | `Ctrl+T`, the status-bar button, or the palette. |
| **Toast notifications** | Non-blocking messages bottom-right (`Toast::notify(this, "Saved", Toast::Success)`). |
| **Status bar** | Signed-in user and role, live clock, theme switch, shortcut hint. |
| **Smoother login** | No blocking "Login Success" dialog; errors shown as toasts. |

## New files
```
ims_theme.h/.cpp     ThemeManager: tokens, QSS, role tagging, chart styling
ims_fx.h/.cpp        Fx: fade, page transition, count-up, hover lift, shake
ims_toast.h/.cpp     Toast notifications
ims_palette.h/.cpp   Ctrl+K command palette
ims_explorer.h/.cpp  Inventory Explorer page
```
`IMS.pro` already lists them. Re-run qmake (or "Run qmake" in Qt Creator) before building.

## Modified files
`main.cpp` (installs the theme, non-blocking splash), `mainwindow.cpp` (toasts, shake),
`dashboard.h/.cpp` (nav highlighting via properties instead of per-click inline styles,
status bar, shortcuts, Explorer page, palette), the three chart widgets (one extra line
each), `IMS.pro`.

## Build
Qt 6 (Widgets, Sql, Charts) and a C++17 compiler, same as before:
```
qmake IMS.pro && make      # or open IMS.pro in Qt Creator and press Run
```

## Customising
* Brand colour / surfaces: edit `ThemeManager::tokens()` in `ims_theme.cpp`.
* A new button style: set `button->setProperty("role", "primary" | "danger" | "ghost")`.
* A new card: `frame->setProperty("role", "card")`.
* Keep a widget out of the automatic styling: set `setProperty("ims_managed", true)` on it.

## Note
This layer was written without access to a Qt toolchain in the authoring environment,
so it has not been compiled or run there. Expect to do a first build and fix any small
compile or visual issues; see the list in the project hand-off notes.
