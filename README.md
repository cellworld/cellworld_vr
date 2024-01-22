
# Cellworld VR

Virtual Reality project for Unreal Engine 4.27 for testing the HP Reverb G2 VR Headset and OmniceptSDK. 

## To do: 

- save user head positions, rotations, etc for eye-tracking analysis (similar to FGetPlayerTraversePath)
- figure out a new calibration method to find depth/actor player is looking at (papers support just cast a ray and use temporal information to infer if the object was focused on or not)
- occlusion spawning: check for black pixel in image and spawn actor there (can be an occlusion). Add unique Id with AActor->SetActorLabel("New_label")
- HMD Tracking and world scaling: Need to get original HMD position in space. PlayerInitHMDPosAndOrientation = WorldOrigin (door of experiment). 

## Omnicept SDK

Very brief sensor description below. Make sure to carefully go over the [G2 system overview](https://developers.hp.com/omnicept/docs/fundamentals#system-overview).

### Eye-Tracking 
Cameras pointing at the user's left and right eye. The eye camera and eye-tracking on the HP Reverb G2 Omnicept eye-tracking uses Tobii systems (to do: add reference link).

### Heart-Rate

PPG (photoplethysmogram) sensor detects the blood volume changes in the microvascular bed of tissue using light signals that reflect onto the skin. The PPG sensor is located on the forehead, represented by two green LEDs.   HP does not provide access to the raw PPG data.  Why?

NOTE: PPG Requires a special agreement in place, PPG is not enabled by default

#### Outputs
From Heart rate we can get the tandard deviation between two normal heartbeats (SDNN) and root-mean square of successive differences (RMSSD).

| Sensor | Output | f_s |
|---|---|---|
| Heart Rate | Beats Per Minute (BPM) with (int) range 40-350 | 5 Hz (5 samp/sec) |
| Heart Rate Variability (HRV) | SDNN and RMSSD (milliseconds) | 0.016 Hz (1 samp/minute) |
| PPG Sensor | Requires special permissions from HP to access | [PPG Information](http://https//developers.hp.com/system/files/attachments/HPO-CLD%20Technical%20Report%204.30.21.pdf) |

#### Caveats & Limitations
Heart Rate Variability takes a minimum of 70 seconds to report a reading. 
If HRV does not have enough data for a reading, the reported value will be 0.
Not intended for use as a clinical device.

## Setup 
Complete description and images of steps are provided in [Getting Started with Omnicept](https://developers.hp.com/omnicept/docs/fundamentals), I recommend you follow these if it is your first time using OmniceptSDK and HPGlia Plugin. 

### tl;dr	
1. Create HP Developer account and then download (and install) [Omnicept Developer SDK](https://developers.hp.com/omnicept/hp-omnicept-sdk).
2. Follow [these steps](https://developers.hp.com/omnicept/docs/console/getting-started) to generate an HP developer license (composed of an client id and access key) in the [HP Console](https://omnicept-console.hpbp.io/).
3. Follow [usage steps](https://developers.hp.com/omnicept/docs/ue4/getting-started#usage) to attach license (remember it's 2 items) values into ```HPGlia Settings ``` from within the Unreal Editor (```Edit > Project Settings > Plugins > HP Glia Settings```).

#### IMPORTANT SECURITY NOTE 
Taken from [Unreal > Getting started](https://developers.hp.com/omnicept/docs/ue4/getting-started): 
```
To prevent Client ID and Access Key values from being readable in a packaged project, enable encryption for initialization
files. Under Edit -> Project Settings -> Project -> Crypto click "Generate New Encryption Key" button, then check the box
for "Encrypt Pak Ini Files".
```
Read more [here](https://developers.hp.com/omnicept/docs/ue4/getting-started#securing).
