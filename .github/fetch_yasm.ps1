$ErrorActionPreference = "Stop"

$project_root = "$psScriptRoot\.."
Write-Output "project_root: $project_root"

$yasm_version = "1.3.0"
$yasm_dlexe = "yasm-$yasm_version-win64.exe"

$yasm_url = "https://github.com/yasm/yasm/releases/download/v$yasm_version/$yasm_dlexe"
$yasm_exename = "yasm.exe"
$yasm_exepath = "$project_root/yasm.exe"

$yasm_dlpath = "$project_root\$yasm_dlexe"

echo "yasm_dlpath:  $yasm_dlpath"
echo "yasm_exename: $yasm_exename"
echo "yasm_exepath: $yasm_exepath"

echo "Cleaning previous artifacts"
if (Test-Path $yasm_dlpath) {
    Remove-Item $yasm_dlpath -Force
}
if (Test-Path $yasm_exepath) {
    Remove-Item $yasm_exepath -Force
}

Write-Output "Downloading $yasm_dlexe ($yasm_url)"
Invoke-WebRequest -Uri $yasm_url -OutFile $yasm_dlpath

Write-Output "Moving $yasm_dlexe to $yasm_exename"
Rename-Item $yasm_dlpath $yasm_exename

Write-Output "Done"
