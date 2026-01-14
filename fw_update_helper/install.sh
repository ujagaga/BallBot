#!/usr/bin/env bash


# --- Installation Section ---
echo "Installing dependencies..."
if ! sudo apt update -y; then
  echo "Error: Failed to update apt repositories. Aborting installation."
  exit 1
fi

if ! sudo apt install -y python3-pip python3-venv; then
  echo "Error: Failed to install dependencies. Aborting installation."
  exit 1
fi

echo "Creating virtual environment..."
python3 -m venv venv
if [ $? -ne 0 ]; then
  echo "Error: Failed to create virtual environment. Aborting installation."
  exit 1
fi

echo "Activating virtual environment..."
source venv/bin/activate
if [ $? -ne 0 ]; then
  echo "Error: Failed to activate virtual environment. Aborting installation."
  exit 1
fi

echo "Installing Python packages..."
pip3 install requests
if [ $? -ne 0 ]; then
  echo "Error: Failed to install python libraries. Aborting installation."
  exit 1
fi

echo "Deactivating virtual environment..."
deactivate