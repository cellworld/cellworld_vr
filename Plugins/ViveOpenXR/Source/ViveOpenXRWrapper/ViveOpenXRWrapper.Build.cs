// Copyright HTC Corporation. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
	public class ViveOpenXRWrapper : ModuleRules
	{
		public ViveOpenXRWrapper(ReadOnlyTargetRules Target) : base(Target)
		{
			Type = ModuleType.External;

			PublicIncludePaths.AddRange(
				new string[] {
					Path.Combine(ModuleDirectory, "Public")
				}
			);
		}
	}
}
