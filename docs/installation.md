# Installation and Configuration

## Installation process

1. Clone [Unreal Engine repo](https://github.com/EpicGames/UnrealEngine/) to your disk
1. Apply [Restricted capabilities PR](https://github.com/EpicGames/UnrealEngine/pull/8284) to the Unreal sources. This is very important, without this PR you can't run your project on the device. How to do so, please refer [the GitHub documentaion](https://docs.github.com/github/collaborating-with-pull-requests/reviewing-changes-in-pull-requests/checking-out-pull-requests-locally).
1. Clone this repo to your disk
1. Build the Unreal Editor from the sources
1. If you are using your own project, copy `this_repo/HL2App/Plugins/HololensResearchMode` folder to your project's plugins. Otherwise you can use the HL2App project.
1. Generate VS solution for a project and build it in Development Editor configuration.
1. Start the editor from your VS solution.
   
## Configuration

### Device Configuration

Please follow [Enabling Research Mode (HoloLens first Gen and HoloLens 2)](https://docs.microsoft.com/en-gb/windows/mixed-reality/develop/platform-capabilities-and-apis/research-mode#enabling-research-mode-hololens-first-gen-and-hololens-2) section from the official Hololens Research Mode page to enable access to sensor streams in Device Portal.

### Unreal Editor Configuration

To get runtime access to sensor streams, please open the Unreal Editor Project configuration.

1. Open **Platforms - Hololens** section
1. If you are using HL2App project, click **Generate New** in **Packaging/Signing Certificate**
1. Go to **Capabilities** and make sure that **Webcam** is enabled 
1. Open **Platforms - Hololens Research Mode** section
1. Go to **Capabilities** and make that **Perception Sensor Experimental** and **Background Spatial Perception** both are enabled.
   
Please make an important note. The **Perception Sensor Experimental** and **Background Spatial Perception** capabilities are invasive and experimental. You can't use them for Windows Store app or for a non-development device. If you need to remove Hololens ResearchMode plugin from your project, you must disable them both at first. Otherwise, they might silently applied to your project configuration.  

The configuration is complete, the next is [Test Project description](test-project-description.md)

## Test Project description

This section is dedicated to HL2App project, which is a sample app using Hololens ResearchMode plugin. The project has four BluePrint actors - Accelerometer, Gyroscope, Magnetometer and CamCapture. 

Performance isn't a point of implementation, the target is to show how to get the data from the sensors. So the first three of them use debug string output to present the results. 

CamCapture is a universal actor for all possible sensor cams. The test project displays four slates with four grey scale Head Tracking cam and one slate with short distance Depth Cam. If you want to see a long distant Depth Cam, you need **World Outliner** in default level, select **Depth cam** and choose **Long Distance Depth Sensor** in **Details/Default/Sensor Type** dropbox.

To start the app you should package the project from editor and install it in Device Portal
