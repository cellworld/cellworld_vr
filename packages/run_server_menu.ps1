# Get the current working directory
$cwd = Get-Location

# Find all first-level directories
$folders = Get-ChildItem -Path $cwd -Directory

# Array to store valid folders
$validFolders = @()

# Loop through each high-level folder
foreach ($folder in $folders) {
    $exePath = Join-Path $folder.FullName "WindowsServer/cellworld_vrServer.exe"

    # Check if the executable exists
    if (Test-Path $exePath) {
        $validFolders += @{ "Name" = $folder.Name; "Path" = $exePath }
    }
}

# Output results and allow user to select a folder
if ($validFolders.Count -gt 0) {
    Write-Host "Available servers to run:"
    for ($i = 0; $i -lt $validFolders.Count; $i++) {
        Write-Host "$($i+1): $($validFolders[$i]['Name'])"
    }

    # Get user input
    $selection = Read-Host "Enter the number of the server you want to run"

    # Validate selection
    if ($selection -match '^\d+$' -and $selection -gt 0 -and $selection -le $validFolders.Count) {
        $selectedServer = $validFolders[$selection - 1]['Path']
        Write-Host "Launching: $selectedServer"

        # Run the selected server executable
        Start-Process -FilePath $selectedServer -ArgumentList "-log" -NoNewWindow
    } else {
        Write-Host "Invalid selection. Exiting..."
    }
} else {
    Write-Host "No folders contain a valid WindowsServer/cellworld_vrServer.exe."
}
