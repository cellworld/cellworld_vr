#!/bin/bash

# Get the current working directory
ProjectPath=$(pwd)

# Define the .uproject file path
UprojectFilePath="$ProjectPath/cellworld_vr.uproject"

# Define the output directory (one level behind ProjectPath and inside "/Packages")
ParentPath=$(dirname "$ProjectPath")
OutputDirectory="$ParentPath/Packages"

# Ensure the output directory exists
mkdir -p "$OutputDirectory"

# Build the command
Command="RunUAT BuildCookRun -project=\"$UprojectFilePath\" -noP4 -platform=Win64 -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive -archivedirectory=\"$OutputDirectory\""

# Execute the command
echo $Command
eval $Command

