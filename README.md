# 🚀 NCommander
A professional, lightweight, retro-style file manager for Linux/Unix systems, written in **Standard C** using the **ncurses** library. Inspired by the legendary Norton Commander.


![preview](/images/ncommander.jpg)
---

## 🧠 Why ncurses? 
This project is built upon the **ncurses (new curses)** library, the industry standard for creating **Text User Interfaces (TUI)** in terminal environments. 

Ncurses was chosen for:
- **Window Management**: Handling the dual-panel layout and pop-up dialog boxes.
- **Color Control**: Rendering the classic blue background and custom color themes.
- **Advanced Input**: Capturing function keys (F1-F10), arrow keys, and backspace, which are not supported by standard `stdio.h`.
- **Screen Optimization**: Utilizing "double buffering" (refresh/wrefresh) to update the terminal instantly without flickering.

---

## ✨ Features
- **Dual-Panel Interface**: Independent navigation in two panels.
- **Multi-Selection**: Use `Space` to mark multiple files for batch operations.
- **File Management**: Copy (F5), Move (F6), Delete (F8), MkDir (F7) with auto-enter.
- **Permissions**: Change file permissions via Chmod (F2).
- **Archiving**: Support for Zip (F9) and Unzip (F10).
- **Search**: Real-time list filtering using `/` or `S`.
- **View & Edit**: Built-in Viewer (F3) and external Editor support (F4/Nano).
- **System Info**: Real-time free disk space indicator.

---

## 🛠 Compilation & Installation

The executable is compiled and builded with gcc 13.3.0

```
make build   # Compiles the project and creates the 'ncommander' binary
make run     # Compiles and executes the application immediately
make clean   # Removes build files and the binary
```

Permanent System Installation

```chmod +x install.sh
./install.sh
```

## ⌨️Keybindings

| Key | Action |	
| - | - |
| F1 | Home Directory |
| F1 | Home Directory |
| F2 | Chmod (Permissions) |
| F3 | View (Internal Viewer) |
| F4 | Edit (Nano/External) |	
| F5 | Copy (Single or Marked) |	
| F6 | Move (Single or Marked) |	
| F7 | MkDir (Create & Enter) |
| F8 | Delete (Single or Marked) |
| F9 | Zip (Compress Items) |
| F10 | Unzip (Extract Archive) |
| Space | Mark/Unmark File |
| Backspace | Go to Parent Dir ".." |
| Tab | Switch Active Panel |	
| S | Search Filter |
| Enter | Open Directory |
| T | Change UI Theme |
| Arrows | Navigate / Switch |
| Q | Exit Application |

### Prerequisites
You must install the ncurses development libraries and build tools:

```bash
sudo apt update
sudo apt install libncurses5-dev libncursesw5-dev gcc make zip unzip
```
