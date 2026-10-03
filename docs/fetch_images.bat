@echo off
rem Downloads the photos from the Printables pages into docs\images\
rem Double-click it or run it from any folder: docs\fetch_images.bat
setlocal
cd /d "%~dp0images" || (mkdir "%~dp0images" && cd /d "%~dp0images")

echo Gauge housing
echo   cover.jpg
curl -sSfL -A "Mozilla/5.0" -o "cover.jpg" "https://media.printables.com/media/prints/e186adaa-03fd-490f-ab2f-dcd577337b5e/images/10422782_16cf345d-969d-4fb7-9edc-316a68af9402_835c6eed-ea62-4689-8dd6-b9f21dd2f6a4/thumbs/cover/1200x630/jpg/img20250617104447.jpg" || echo     FAILED: cover.jpg
echo   housing.webp
curl -sSfL -A "Mozilla/5.0" -o "housing.webp" "https://media.printables.com/media/prints/1326148/rich_content/7f9e398a-2bd5-4322-b280-8a6ba254b7e2/thumbs/cover/800x1067/jpg/img20250616094015.webp" || echo     FAILED: housing.webp
echo Cable add-on
echo   cable-addon.webp
curl -sSfL -A "Mozilla/5.0" -o "cable-addon.webp" "https://media.printables.com/media/prints/1326128/rich_content/c879ad83-2058-47fa-8fce-896686e2fd70/thumbs/cover/800x575/jpg/img20250520133031.webp" || echo     FAILED: cable-addon.webp

echo Done. Files are in %CD%
pause
