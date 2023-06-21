# Microsoft Hololens 2 Research Mode for Unreal Engine

## What is the Microsoft Hololens 2 Research Mode plugin?
The Microsoft Hololens 2 Research Mode plugin is an Unreal Engine game plugin which provides access to key sensors available on Microsoft's HoloLens 2 for research applications.

| NOTE: The Microsoft Hololens 2 Research Mode plugin requires **Unreal 4.27.0+**. |
| --- |

## Feature Summary

The full feature list available is on the [HoloLens Research Mode
docs](https://docs.microsoft.com/windows/mixed-reality/develop/platform-capabilities-and-apis/research-mode)

The plugin can be used with the [Microsoft-OpenXR](https://github.com/microsoft/Microsoft-OpenXR-Unreal) or built-in Windows Mixed Reality plugins.

## Installation

1. Install Unreal Engine from [source](https://github.com/EpicGames/UnrealEngine/). If you have never accessed Unreal Engine's source code before, instructions for joining the Epic Games organization on GitHub can be found [here](https://www.unrealengine.com/en-US/ue4-on-github?sessionInvalidated=true).
1. Download the Microsoft Microsoft Hololens 2 Research Mode plugin from the this repo.
1. Apply this restricted capabilities PR for [UE4](https://github.com/EpicGames/UnrealEngine/pull/8284) or [UE5](https://github.com/EpicGames/UnrealEngine/pull/10563) into your UE engine sources. Without this PR, a packaged app with 
the required capabilities can't be run on device.
1. Use your own HoloLens 2 project or the provided sample project. 

## Documenation

Links to further documentation can be found [here](docs/index.md)

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft 
trademarks or logos is subject to and must follow 
[Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general).
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.
