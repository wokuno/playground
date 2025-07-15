#!/usr/bin/env python3
"""
GitHub Actions Artifact Downloader
Downloads the latest weather-lstm-artifacts from GitHub Actions runs
"""

import argparse
import json
import os
import sys
import zipfile
import tempfile
from urllib.request import urlopen, Request
from urllib.parse import urljoin
import subprocess

class GitHubArtifactDownloader:
    def __init__(self, repo_owner, repo_name, token=None):
        self.repo_owner = repo_owner
        self.repo_name = repo_name
        self.token = token
        self.base_url = "https://api.github.com"
        
    def _make_request(self, url):
        """Make an authenticated request to GitHub API"""
        headers = {
            'Accept': 'application/vnd.github.v3+json',
            'User-Agent': 'Weather-LSTM-Downloader/1.0'
        }
        
        if self.token:
            headers['Authorization'] = f'token {self.token}'
        
        request = Request(url, headers=headers)
        
        try:
            response = urlopen(request)
            return json.loads(response.read().decode('utf-8'))
        except Exception as e:
            print(f"Error making request to {url}: {e}")
            return None
    
    def get_workflow_runs(self, workflow_name="train-and-predict.yml", limit=10):
        """Get recent workflow runs"""
        url = f"{self.base_url}/repos/{self.repo_owner}/{self.repo_name}/actions/workflows/{workflow_name}/runs"
        url += f"?per_page={limit}&status=success"
        
        return self._make_request(url)
    
    def get_run_artifacts(self, run_id):
        """Get artifacts for a specific run"""
        url = f"{self.base_url}/repos/{self.repo_owner}/{self.repo_name}/actions/runs/{run_id}/artifacts"
        return self._make_request(url)
    
    def download_artifact(self, artifact_url, output_path):
        """Download an artifact"""
        headers = {
            'Accept': 'application/vnd.github.v3+json',
            'User-Agent': 'Weather-LSTM-Downloader/1.0'
        }
        
        if self.token:
            headers['Authorization'] = f'token {self.token}'
        
        request = Request(artifact_url, headers=headers)
        
        try:
            response = urlopen(request)
            
            # GitHub returns a redirect to the actual download URL
            actual_url = response.geturl()
            
            # Download the actual file
            actual_request = Request(actual_url)
            actual_response = urlopen(actual_request)
            
            with open(output_path, 'wb') as f:
                f.write(actual_response.read())
            
            return True
        except Exception as e:
            print(f"Error downloading artifact: {e}")
            return False
    
    def extract_artifact(self, zip_path, extract_to):
        """Extract downloaded artifact zip file"""
        try:
            with zipfile.ZipFile(zip_path, 'r') as zip_ref:
                zip_ref.extractall(extract_to)
            return True
        except Exception as e:
            print(f"Error extracting artifact: {e}")
            return False
    
    def find_latest_artifact(self, artifact_name="weather-lstm-artifacts"):
        """Find the latest artifact from successful runs"""
        runs_data = self.get_workflow_runs()
        
        if not runs_data or 'workflow_runs' not in runs_data:
            print("No workflow runs found")
            return None
        
        for run in runs_data['workflow_runs']:
            if run['status'] == 'completed' and run['conclusion'] == 'success':
                print(f"Checking run {run['id']} from {run['created_at']}")
                
                artifacts_data = self.get_run_artifacts(run['id'])
                
                if artifacts_data and 'artifacts' in artifacts_data:
                    for artifact in artifacts_data['artifacts']:
                        if artifact['name'] == artifact_name:
                            return {
                                'run_id': run['id'],
                                'artifact': artifact,
                                'run_date': run['created_at']
                            }
        
        return None

def get_repo_info():
    """Try to determine repository info from git remote"""
    try:
        # Get the remote URL
        result = subprocess.run(['git', 'remote', 'get-url', 'origin'], 
                              capture_output=True, text=True, check=True)
        remote_url = result.stdout.strip()
        
        # Parse GitHub URL
        if 'github.com' in remote_url:
            if remote_url.startswith('git@'):
                # SSH format: git@github.com:owner/repo.git
                parts = remote_url.replace('git@github.com:', '').replace('.git', '').split('/')
            else:
                # HTTPS format: https://github.com/owner/repo.git
                parts = remote_url.replace('https://github.com/', '').replace('.git', '').split('/')
            
            if len(parts) >= 2:
                return parts[0], parts[1]
    
    except subprocess.CalledProcessError:
        pass
    
    return None, None

def main():
    parser = argparse.ArgumentParser(description='Download GitHub Actions artifacts')
    parser.add_argument('--owner', help='Repository owner (auto-detected from git if not provided)')
    parser.add_argument('--repo', help='Repository name (auto-detected from git if not provided)')
    parser.add_argument('--token', help='GitHub personal access token (optional for public repos)')
    parser.add_argument('--artifact-name', default='weather-lstm-artifacts',
                       help='Name of the artifact to download (default: weather-lstm-artifacts)')
    parser.add_argument('--output-dir', default='.',
                       help='Directory to extract artifacts to (default: current directory)')
    parser.add_argument('--list-runs', action='store_true',
                       help='List recent workflow runs and exit')
    
    args = parser.parse_args()
    
    # Try to auto-detect repository info
    if not args.owner or not args.repo:
        auto_owner, auto_repo = get_repo_info()
        if auto_owner and auto_repo:
            args.owner = args.owner or auto_owner
            args.repo = args.repo or auto_repo
            print(f"Auto-detected repository: {args.owner}/{args.repo}")
        else:
            print("Could not auto-detect repository. Please specify --owner and --repo")
            return 1
    
    # Check for GitHub token in environment if not provided
    if not args.token:
        args.token = os.environ.get('GITHUB_TOKEN')
    
    downloader = GitHubArtifactDownloader(args.owner, args.repo, args.token)
    
    if args.list_runs:
        # List recent runs
        runs_data = downloader.get_workflow_runs()
        if runs_data and 'workflow_runs' in runs_data:
            print("Recent workflow runs:")
            for run in runs_data['workflow_runs'][:5]:
                status_emoji = "✅" if run['conclusion'] == 'success' else "❌"
                print(f"  {status_emoji} Run {run['id']} - {run['created_at']} - {run['conclusion']}")
        return 0
    
    # Find and download latest artifact
    print(f"Looking for latest '{args.artifact_name}' artifact...")
    
    latest = downloader.find_latest_artifact(args.artifact_name)
    
    if not latest:
        print(f"No '{args.artifact_name}' artifacts found in recent successful runs")
        return 1
    
    artifact = latest['artifact']
    run_id = latest['run_id']
    
    print(f"Found artifact from run {run_id} ({latest['run_date']})")
    print(f"Artifact: {artifact['name']} ({artifact['size_in_bytes']} bytes)")
    
    # Download artifact
    with tempfile.NamedTemporaryFile(suffix='.zip', delete=False) as tmp_file:
        tmp_path = tmp_file.name
    
    print(f"Downloading artifact...")
    
    if downloader.download_artifact(artifact['archive_download_url'], tmp_path):
        print(f"Downloaded to temporary file: {tmp_path}")
        
        # Extract artifact
        print(f"Extracting to {args.output_dir}...")
        
        if downloader.extract_artifact(tmp_path, args.output_dir):
            print(f"Successfully extracted artifact to {args.output_dir}")
            
            # Show what was extracted
            extracted_files = []
            for root, dirs, files in os.walk(args.output_dir):
                for file in files:
                    if file not in ['.gitignore', 'README.md']:  # Skip common files
                        rel_path = os.path.relpath(os.path.join(root, file), args.output_dir)
                        extracted_files.append(rel_path)
            
            if extracted_files:
                print("Extracted files:")
                for file in sorted(extracted_files):
                    print(f"  {file}")
            
            # Clean up temp file
            os.unlink(tmp_path)
            
            return 0
        else:
            print("Failed to extract artifact")
            os.unlink(tmp_path)
            return 1
    else:
        print("Failed to download artifact")
        os.unlink(tmp_path)
        return 1

if __name__ == '__main__':
    sys.exit(main())
