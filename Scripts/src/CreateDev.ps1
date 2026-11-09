$dest = "externals/generated/Lib/Development"

if (!(Test-Path $dest)) {
    New-Item -ItemType Directory -Path $dest | Out-Null
}

Copy-Item -Path "externals/generated/Lib/Release/*" `
          -Destination $dest `
          -Recurse -Force