param(
	[string]$InstalledIncludeRoot = "bin/Debug/inc"
)

$ErrorActionPreference = "Stop"

$repositoryRoot = Resolve-Path (Join-Path $PSScriptRoot "../..")
$rootPath = $InstalledIncludeRoot
if (-not [System.IO.Path]::IsPathRooted($rootPath)) {
	$rootPath = Join-Path $repositoryRoot $rootPath
}

if (-not (Test-Path -LiteralPath $rootPath)) {
	Write-Error "Installed include root does not exist: $rootPath"
}

$resolvedRoot = (Resolve-Path -LiteralPath $rootPath).Path
$violations = New-Object System.Collections.Generic.List[string]
$headers = Get-ChildItem -LiteralPath $resolvedRoot -Recurse -File -Filter "*.h"
$expectedHeaders = @(
	"AssetSuite/AssetSuite.h",
	"AssetSuite/AssetSuiteDescriptors.h",
	"AssetSuite/AssetSuiteExport.h",
	"AssetSuite/AssetSuiteHandles.h",
	"AssetSuite/AssetSuiteTypes.h"
)
$seenHeaders = New-Object "System.Collections.Generic.HashSet[string]"

if ($headers.Count -eq 0) {
	$violations.Add("${resolvedRoot}: no installed public headers were found")
}

foreach ($header in $headers) {
	$relativePath = $header.FullName.Substring($resolvedRoot.Length).TrimStart("\", "/").Replace("\", "/")
	[void]$seenHeaders.Add($relativePath)
	if ($relativePath -notmatch '^AssetSuite[\\/][^\\/]+\.h$') {
		$violations.Add("${header}: installed header must live under AssetSuite/*.h")
	}
}

foreach ($expectedHeader in $expectedHeaders) {
	if (-not $seenHeaders.Contains($expectedHeader)) {
		$violations.Add("${resolvedRoot}: missing expected public header $expectedHeader")
	}
}

foreach ($seenHeader in $seenHeaders) {
	if ($expectedHeaders -notcontains $seenHeader) {
		$violations.Add("${resolvedRoot}: unexpected installed public header $seenHeader")
	}
}

if ($violations.Count -gt 0) {
	Write-Host "Public install surface violations:"
	foreach ($violation in $violations) {
		Write-Host "  $violation"
	}
	exit 1
}

Write-Host "Public install surface check passed for $($headers.Count) headers."
