@echo off
setlocal
cd /d "%~dp0..\.."

set URL=http://127.0.0.1:8765
set PYTHON_EXE=C:\Users\abcc0\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe

if not exist "%PYTHON_EXE%" set PYTHON_EXE=python

powershell.exe -NoProfile -Command "try { Invoke-WebRequest -UseBasicParsing 'http://127.0.0.1:8765/api/feedback' -TimeoutSec 1 | Out-Null; exit 0 } catch { exit 1 }" >nul 2>nul
if %ERRORLEVEL% EQU 0 goto already_running

echo Starting PixelEngine Feedback Dashboard...
echo URL: %URL%
echo.
start "" "%URL%"
"%PYTHON_EXE%" "Tools\FeedbackDashboard\server.py" --host 127.0.0.1 --port 8765
echo.
echo Dashboard server stopped.
echo If you see an error above, send it to Codex.
pause
exit /b

:already_running
echo Dashboard is already running.
echo Opening browser: %URL%
start "" "%URL%"
timeout /t 2 >nul
exit /b
