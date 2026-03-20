#!/bin/bash

echo "🚀 Starting NCommander installation..."

# Check for ncurses dependency
if ! pkg-config --exists ncurses; then
    echo "❌ Error: ncurses library not found."
    echo "Please install it: sudo apt install libncurses5-dev libncursesw5-dev"
    exit 1
fi

# Build the project
echo "🛠 Compiling..."
make build

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful."
    # Move to /usr/local/bin
    echo "📂 Installing to /usr/local/bin/nc (requires sudo)..."
    sudo cp ncommander /usr/local/bin/nc
    sudo chmod +x /usr/local/bin/nc
    echo "🎉 Installation complete! You can now run the app by typing 'nc'."
else
    echo "❌ Compilation failed."
    exit 1
fi
