param(
	[string[]]$Roots = @(
		"include/AssetSuite",
		"bin/Debug/inc",
		"bin/Release/inc"
	)
)

$ErrorActionPreference = "Stop"

$repositoryRoot = Resolve-Path (Join-Path $PSScriptRoot "../..")
$patterns = [ordered]@{
	"Windows.h include" = '(?i)#\s*include\s*[<"]windows\.h[>"]'
	"Win32 alias type" = '\b(?:BYTE|FLOAT|UINT)\b'
	"STL owning API type" = '\bstd\s*::\s*(?:vector|string|basic_string)\b|\bstd\s*::\s*filesystem\s*::\s*path\b'
	"Legacy manager or decoder surface" = '\b(?:Manager|Decoder|ImageDecoder|MeshDecoder|ImageDecoders|MeshDecoders)\b'
}

$violations = New-Object System.Collections.Generic.List[string]
$checkedHeaders = 0
$expandedRoots = @()

foreach ($root in $Roots) {
	$expandedRoots += $root -split ","
}

foreach ($root in $expandedRoots) {
	if ([string]::IsNullOrWhiteSpace($root)) {
		continue
	}

	$root = $root.Trim()
	$rootPath = $root
	if (-not [System.IO.Path]::IsPathRooted($rootPath)) {
		$rootPath = Join-Path $repositoryRoot $rootPath
	}

	if (-not (Test-Path -LiteralPath $rootPath)) {
		continue
	}

	$resolvedRoot = (Resolve-Path -LiteralPath $rootPath).Path
	$isInstalledIncludeRoot = (Split-Path -Leaf $resolvedRoot) -ieq "inc"
	$headers = Get-ChildItem -LiteralPath $resolvedRoot -Recurse -File -Filter "*.h"

	foreach ($header in $headers) {
		$checkedHeaders++
		$relativePath = $header.FullName.Substring($resolvedRoot.Length).TrimStart("\", "/")

		if ($isInstalledIncludeRoot -and ($relativePath -notmatch '^AssetSuite[\\/][^\\/]+\.h$')) {
			$violations.Add("${header}: installed header must live under AssetSuite/*.h")
		}

		$lineNumber = 0
		foreach ($line in [System.IO.File]::ReadLines($header.FullName)) {
			$lineNumber++
			foreach ($pattern in $patterns.GetEnumerator()) {
				if ($line -match $pattern.Value) {
					$violations.Add("${header}:${lineNumber}: $($pattern.Key): $line")
				}
			}
		}
	}
}

if ($checkedHeaders -eq 0) {
	Write-Error "No public headers were found to audit."
}

if ($violations.Count -gt 0) {
	Write-Host "Public header hygiene violations:"
	foreach ($violation in $violations) {
		Write-Host "  $violation"
	}
	exit 1
}

Write-Host "Public header hygiene check passed for $checkedHeaders headers."
