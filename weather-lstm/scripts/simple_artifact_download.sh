#!/bin/bash
# Simple artifact downloader using curl and GitHub API
# This is a fallback when Python script and GitHub CLI fail

set -e

REPO_OWNER="$1"
REPO_NAME="$2"
GITHUB_TOKEN="$3"
ARTIFACT_NAME="${4:-weather-lstm-artifacts}"

if [ -z "$REPO_OWNER" ] || [ -z "$REPO_NAME" ] || [ -z "$GITHUB_TOKEN" ]; then
    echo "Usage: $0 <owner> <repo> <token> [artifact_name]"
    echo "❌ Missing required parameters"
    exit 1
fi

echo "🔍 Looking for artifacts in $REPO_OWNER/$REPO_NAME"

# Get recent successful workflow runs
RUNS_JSON=$(curl -s -H "Authorization: token $GITHUB_TOKEN" \
    -H "Accept: application/vnd.github.v3+json" \
    "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/actions/workflows/weather-lstm.yml/runs?status=success&per_page=5")

if [ $? -ne 0 ]; then
    echo "❌ Failed to fetch workflow runs"
    exit 1
fi

# Extract run IDs
RUN_IDS=$(echo "$RUNS_JSON" | jq -r '.workflow_runs[]?.id // empty' 2>/dev/null)

if [ -z "$RUN_IDS" ]; then
    echo "ℹ️  No successful runs found"
    exit 0
fi

echo "📋 Found runs: $(echo $RUN_IDS | tr '\n' ' ')"

# Try each run until we find one with artifacts
for RUN_ID in $RUN_IDS; do
    echo "🔍 Checking run $RUN_ID for artifacts..."
    
    ARTIFACTS_JSON=$(curl -s -H "Authorization: token $GITHUB_TOKEN" \
        -H "Accept: application/vnd.github.v3+json" \
        "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/actions/runs/$RUN_ID/artifacts")
    
    if [ $? -ne 0 ]; then
        echo "⚠️  Failed to fetch artifacts for run $RUN_ID"
        continue
    fi
    
    # Look for our specific artifact
    ARTIFACT_URL=$(echo "$ARTIFACTS_JSON" | jq -r ".artifacts[]? | select(.name == \"$ARTIFACT_NAME\") | .archive_download_url" 2>/dev/null)
    
    if [ -n "$ARTIFACT_URL" ] && [ "$ARTIFACT_URL" != "null" ]; then
        echo "✅ Found artifact: $ARTIFACT_NAME"
        echo "📥 Download URL: $ARTIFACT_URL"
        
        # Download the artifact
        echo "⬇️  Downloading artifact..."
        if curl -L -H "Authorization: token $GITHUB_TOKEN" \
            -H "Accept: application/vnd.github.v3+json" \
            -o "artifact.zip" \
            "$ARTIFACT_URL"; then
            
            echo "✅ Downloaded artifact successfully"
            
            # Extract the artifact
            if command -v unzip >/dev/null 2>&1; then
                echo "📦 Extracting artifact..."
                unzip -q "artifact.zip" 2>/dev/null || unzip "artifact.zip"
                rm -f "artifact.zip"
                echo "✅ Artifact extracted successfully"
                
                # Show what was extracted
                echo "📂 Extracted contents:"
                find . -name "*.bin" -o -name "*.csv" -o -name "train" -o -name "predict" | sort
                
                exit 0
            else
                echo "❌ unzip command not available"
                rm -f "artifact.zip"
                exit 1
            fi
        else
            echo "❌ Failed to download artifact"
            exit 1
        fi
    else
        echo "ℹ️  No '$ARTIFACT_NAME' artifact found in run $RUN_ID"
    fi
done

echo "ℹ️  No artifacts found in any recent successful runs"
exit 0
