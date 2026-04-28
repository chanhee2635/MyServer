$protoDir    = $PSScriptRoot
$solutionDir = Resolve-Path "$protoDir\..\.."

$packetMap = @{
    "Game"  = @{ Project = "GameServer";  Handler = "ClientPacketHandler"; SessionType = "GameSession"  }
    "Login" = @{ Project = "LoginServer"; Handler = "LoginPacketHandler";  SessionType = "LoginSession" }
}

# C_ENTER → CEnter
function EnumToClass($enumName) {
    $parts = $enumName -split '_'
    return ($parts | ForEach-Object {
        $_.Substring(0,1).ToUpper() + $_.Substring(1).ToLower()
    }) -join ''
}

foreach ($proto in $packetMap.Keys)
{
    $info        = $packetMap[$proto]
    $protoFile   = Join-Path $protoDir "$proto.proto"
    $projectDir  = Join-Path $solutionDir $info.Project
    $outputDir   = Join-Path $projectDir "Protocol"
    $handlerName = $info.Handler
    $sessionType = $info.SessionType
    $sessionRef  = "${sessionType}Ref"

    if (-not (Test-Path $protoFile)) {
        Write-Warning "[$proto] $protoFile not found, skipping"
        continue
    }

    if (-not (Test-Path $projectDir)) {
        Write-Warning "[$proto] Project folder not found ($projectDir), skipping"
        continue
    }

    # C_ packet parsing
    $lines         = Get-Content $protoFile
    $clientPackets = [System.Collections.Generic.List[string]]::new()

    foreach ($line in $lines) {
        if ($line -match '^\s+(C_\w+)\s*=\s*\d+') {
            $clientPackets.Add($matches[1])
        }
    }

    if ($clientPackets.Count -eq 0) {
        Write-Warning "[$proto] No C_ packets found, skipping"
        continue
    }

    Write-Host "[$proto] $($clientPackets.Count) client packet(s) found" -ForegroundColor Cyan

    if (-not (Test-Path $outputDir)) {
        New-Item -ItemType Directory -Path $outputDir | Out-Null
    }

    # ── Header ────────────────────────────────────────────────
    $onHandleDecls = ($clientPackets | ForEach-Object {
        $cls = EnumToClass $_
        "    static bool OnHandle_$_($sessionRef session, const Protocol::${cls}& pkt);"
    }) -join "`n"

    $headerContent = @"
// AUTO-GENERATED -- DO NOT EDIT MANUALLY
// Source: $proto.proto  /  Run Gen.bat to regenerate
#pragma once
#include "$proto.pb.h"
#include "${sessionType}.h"

template<auto PacketType, typename T>
inline SendBufferRef MakeSendBuffer(const T& msg)
{
    const uint32 bodySize  = static_cast<uint32>(msg.ByteSizeLong());
    const uint32 totalSize = sizeof(PacketHeader) + bodySize;

    SendBufferRef sendBuffer = GSendBufferManager->Open(totalSize);

    PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->GetBuffer());
    header->size = static_cast<uint16>(totalSize);
    header->type = static_cast<uint16>(PacketType);

    msg.SerializeToArray(sendBuffer->GetBuffer() + sizeof(PacketHeader), static_cast<int>(bodySize));
    sendBuffer->Close(totalSize);
    return sendBuffer;
}

class $handlerName
{
public:
    static bool Handle($sessionRef session, std::span<const BYTE> packet, uint16 type);

$onHandleDecls

private:
    template<typename MsgType, bool(*OnHandle)($sessionRef, const MsgType&)>
    static bool HandlePacket($sessionRef session, std::span<const BYTE> packet)
    {
        MsgType pkt;
        if (!pkt.ParseFromArray(
                packet.data() + sizeof(PacketHeader),
                static_cast<int>(packet.size() - sizeof(PacketHeader))))
            return false;
        return OnHandle(session, pkt);
    }
};
"@

    # ── _Generated.cpp ────────────────────────────────────────
    $switchCases = ($clientPackets | ForEach-Object {
        $cls = EnumToClass $_
        "    case Protocol::$_`: return HandlePacket<Protocol::$cls, OnHandle_$_>(session, packet);"
    }) -join "`n"

    $generatedContent = @"
// AUTO-GENERATED -- DO NOT EDIT MANUALLY
// Source: $proto.proto  /  Run Gen.bat to regenerate
#include "pch.h"
#include "${handlerName}.h"

bool ${handlerName}::Handle($sessionRef session, std::span<const BYTE> packet, uint16 type)
{
    switch (type)
    {
$switchCases
    default: 
        LOG_WARN(\"Unknown packet type=\" + std::to_string(type));
        return false;
    }
}
"@

    # ── Stub .cpp (only if not exists) ────────────────────────
    $stubPath = Join-Path $projectDir "${handlerName}.cpp"

    if (-not (Test-Path $stubPath)) {
        $stubBodies = ($clientPackets | ForEach-Object {
            $cls = EnumToClass $_
@"

bool ${handlerName}::OnHandle_$_($sessionRef session, const Protocol::${cls}& pkt)
{
    return true;
}
"@
        }) -join ""

        @"
#include "pch.h"
#include "${handlerName}.h"
$stubBodies
"@ | Out-File $stubPath -Encoding utf8

        Write-Host "  Stub created: ${handlerName}.cpp" -ForegroundColor Yellow
    }

    # ── Output ────────────────────────────────────────────────
    $headerContent    | Out-File (Join-Path $outputDir "${handlerName}.h")             -Encoding utf8
    $generatedContent | Out-File (Join-Path $outputDir "${handlerName}_Generated.cpp") -Encoding utf8

    Write-Host "  -> $($info.Project)\Protocol\${handlerName}.h"            -ForegroundColor Green
    Write-Host "  -> $($info.Project)\Protocol\${handlerName}_Generated.cpp" -ForegroundColor Green
}

Write-Host "`nDone." -ForegroundColor Yellow
