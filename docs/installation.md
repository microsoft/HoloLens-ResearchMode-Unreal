# Installation and Configuration

## Installation Steps

1. Clone the [Unreal Engine repo](https://github.com/EpicGames/UnrealEngine/). If you have never accessed Unreal Engine's source code before, instructions for joining the Epic Games organization on GitHub can be found [here](https://www.unrealengine.com/en-US/ue4-on-github?sessionInvalidated=true).
1. Apply this [restricted capabilities PR](https://github.com/EpicGames/UnrealEngine/pull/8284) to your Unreal Engine source code. This is very important- without this PR you won't be able to run your project on device. For guidance on applying a PR to a codebase, please refer to GitHub's [documentation](https://docs.github.com/github/collaborating-with-pull-requests/reviewing-changes-in-pull-requests/checking-out-pull-requests-locally).
1. Clone this repo 
1. Build the Unreal Engine Editor from source
1. If you are using research mode in your own project, copy the `<THIS_REPO>/HL2App/Plugins/HololensResearchMode` folder to your project's Plugins folder. Otherwise you can use the HL2App project.
1. Generate a VS solution for your project and build it in the Development Editor configuration.
1. Start the editor from your project's VS solution.
   
## Configuration

### Device Configuration

Please follow the instructions in [Enabling Research Mode (HoloLens First Gen and HoloLens 2)](https://docs.microsoft.com/en-gb/windows/mixed-reality/develop/platform-capabilities-and-apis/research-mode#enabling-research-mode-hololens-first-gen-and-hololens-2) from the official HoloLens Research Mode docs to enable access to sensor streams in Device Portal.

### Project Configuration

To get runtime access to sensor streams, you'll need to configure your project settings. Navigate to **Edit > Project Settings**.

1. Navigate to the **Platforms - Hololens** section
1. If you are using the provided HL2App project, under **Packaging/Signing Certificate**, click **Generate New**
1. Scroll down to the **Capabilities** section and make sure that **Webcam** is enabled 
1. Navigate to the **Platforms - Hololens Research Mode** section
1. Under **Capabilities**, ensure that **Perception Sensor Experimental** and **Background Spatial Perception** are both enabled.
   
NOTE: The **Perception Sensor Experimental** and **Background Spatial Perception** capabilities are invasive and experimental. You can't use them in apps published to the Windows Store or on a non-development device. If you need to remove the HoloLens Research Mode plugin from your project, you must disable both restricted capabilities first. Otherwise, they might get silently applied to your project configuration. 

Your project is now configured for research mode! 

## About the HL2App Sample Project

The HL2App project is a sample project that uses the HoloLens Research Mode plugin and demonstrates how to get data from HoloLens 2's sensors. The project has four Blueprint actors - Accelerometer, Gyroscope, Magnetometer and CamCapture. 

The first three Blueprint actors use debug string output to present the results. CamCapture is a universal actor capable of accessing all possible HoloLens 2 cameras. The HL2App sample app displays four slates with greyscale Head Tracking cameras and one slate with a short distance depth camera. If you want to see a long distance depth camera, navigate to the **World Outliner** in the default level, select **Depth cam** and choose **Long Distance Depth Sensor** from the **Details/Default/Sensor Type** dropbox.

To run the app on device, package the project from the editor and install it via Device Portal.
