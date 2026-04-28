$protoc      = "C:\Projects\vcpkg\installed\x64-windows\tools\protobuf\protoc.exe"
$protoDir    = $PSScriptRoot
$solutionDir = Resolve-Path "$protoDir\..\.."

if (-not (Test-Path $protoc)) {
    Write-Error "protoc.exe not found: $protoc"
    exit 1
}

$deployMap = @{
    "Common" = @("GameServer", "LoginServer", "DummyClient")
    "Game"   = @("GameServer", "DummyClient")
    "Login"  = @("LoginServer", "DummyClient")
}

$generated = @()

foreach ($proto in $deployMap.Keys)
{
    $protoFile = Join-Path $protoDir "$proto.proto"
    $pbHeader  = Join-Path $protoDir "$proto.pb.h"

    if ((Test-Path $pbHeader) -and
        (Get-Item $pbHeader).LastWriteTime -ge (Get-Item $protoFile).LastWriteTime)
    {
        Write-Host "[$proto] Up to date, skipping" -ForegroundColor Gray
        continue
    }

    Write-Host "[$proto] Generating..." -ForegroundColor Cyan
    & $protoc "--proto_path=$protoDir" "--cpp_out=$protoDir" "$protoFile"

    if ($LASTEXITCODE -ne 0) {
        Write-Error "[$proto] Generation failed"
        exit 1
    }

    $generated += $proto
}

foreach ($proto in $generated)
{
    foreach ($project in $deployMap[$proto])
    {
        $destDir = Join-Path (Join-Path $solutionDir $project) "Protocol"

        if (-not (Test-Path $destDir)) {
            New-Item -ItemType Directory -Path $destDir | Out-Null
        }

        Copy-Item "$protoDir\$proto.pb.h"  $destDir -Force
        Copy-Item "$protoDir\$proto.pb.cc" $destDir -Force

        Write-Host "  -> $project\Protocol" -ForegroundColor Green
    }

    Remove-Item "$protoDir\$proto.pb.h"  -Force
    Remove-Item "$protoDir\$proto.pb.cc" -Force
}

Write-Host "`nDone." -ForegroundColor Yellow
