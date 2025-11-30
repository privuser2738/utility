#!/bin/bash
# Script to make all public GitHub repos private for a specific user
# Requires: GitHub CLI (gh) to be installed and authenticated

USERNAME="privuser2738"

echo "Fetching public repositories for $USERNAME..."

# Get all public repos for the user
repos=$(gh repo list "$USERNAME" --visibility public --json name --jq '.[].name' --limit 1000)

if [ -z "$repos" ]; then
    echo "No public repositories found for $USERNAME"
    exit 0
fi

echo "Found the following public repositories:"
echo "$repos"
echo ""
read -p "Do you want to make all these repositories private? (y/N): " confirm

if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
    echo "Operation cancelled."
    exit 0
fi

echo ""
echo "Making repositories private..."

while IFS= read -r repo; do
    if [ -n "$repo" ]; then
        echo "Setting $USERNAME/$repo to private..."
        gh repo edit "$USERNAME/$repo" --visibility private --accept-visibility-change-consequences
        if [ $? -eq 0 ]; then
            echo "  ✓ Successfully made $repo private"
        else
            echo "  ✗ Failed to make $repo private"
        fi
    fi
done <<< "$repos"

echo ""
echo "Done!"
