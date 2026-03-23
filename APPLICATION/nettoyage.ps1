# ===================================================
#   RANGEMENT DU PROJET POUR LE JURY (E6) - V2
# ===================================================

Write-Host "===================================================" -ForegroundColor Cyan
Write-Host "  LANCEMENT DU NETTOYAGE INDUSTRIEL DE L'ARBORESCENCE" -ForegroundColor Cyan
Write-Host "===================================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "[1/3] Creation du dossier d'archivage..." -ForegroundColor Yellow
$archiveDir = "Archives_Tests"
if (-not (Test-Path $archiveDir)) {
    New-Item -ItemType Directory -Path $archiveDir | Out-Null
}

Write-Host "[2/3] Clarification de la documentation..." -ForegroundColor Yellow
if (Test-Path "DOCUMENTATION") { 
    Rename-Item -Path "DOCUMENTATION" -NewName "Docs_Techniques" -ErrorAction SilentlyContinue 
    Write-Host "      -> DOCUMENTATION renomme en Docs_Techniques" -ForegroundColor DarkGray
}

Write-Host "[3/3] Nettoyage des brouillons..." -ForegroundColor Yellow

$foldersToArchive = @(
    "test_codes",
    "testModbusRtuRS485_WT32",
    "ModbusTCPServer_opta_finder_codeArdu*",
    "Autoclave-Raynal-Roquelaure_code_autoclave",
    "Autoclave CODE ESP32",
    "Automate OPTA Finder"
)

$protectedFolders = @("APPLICATION", "OPTA_Master_SCADA", "ESP32_Slaves_Autoclaves", "Docs_Techniques", "Archives_Tests", ".vscode")

foreach ($folder in $foldersToArchive) {
    $paths = Get-ChildItem -Path . -Filter $folder -Directory -ErrorAction SilentlyContinue
    foreach ($p in $paths) {
        if ($protectedFolders -notcontains $p.Name) {
            Write-Host "      -> Archivage de $($p.Name)" -ForegroundColor DarkGray
            Move-Item -Path $p.FullName -Destination $archiveDir -Force -ErrorAction SilentlyContinue
        }
    }
}

Write-Host ""
Write-Host "===================================================" -ForegroundColor Green
Write-Host " OK ! Ton projet est 100% propre et structure." -ForegroundColor Green
Write-Host "===================================================" -ForegroundColor Green

Start-Sleep -Seconds 5